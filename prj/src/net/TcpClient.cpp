#include "PrecHeader.h"
#include "TcpClient.ipp"
////////////////////////////////////////////////////////////////////////////////


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
void LoadBalancer::update_address(IN eco::net::AddressSet& addr)
{
	// reset flag.
	for (auto it = m_address_set.begin(); it != m_address_set.end(); ++it)
	{
		it->m_flag = 0;
	}

	// flag new, old, delete.
	for (auto it = addr.begin(); it != addr.end(); ++it)
	{
		auto it_find = std::find(
			m_address_set.begin(), m_address_set.end(), *it);
		if (it_find == m_address_set.end())
		{
			AddressLoad load(*it);
			load.m_flag = 1;					// #.new
			m_address_set.push_back(load);
		}
		else
		{
			it_find->m_flag = 2;				// #.old
		}
	}

	// delete address and peer.
	for (auto it = m_address_set.begin(); it != m_address_set.end();)
	{
		if (it->m_flag != 0)
		{
			++it;
			continue;
		}
		it = m_address_set.erase(it);			// #.delete.
		if (m_address_cur == *it)
		{
			m_address_cur.clear();
			m_peer->close();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
void LoadBalancer::load_server()
{
	if (m_peer == nullptr)
	{
		auto* io_srv = (IoService*)m_client->m_worker.get_io_service();
		m_peer = TcpPeer::make(io_srv, m_client);
	}

	// this client has connect to server.
	if (m_peer->get_state().is_connected())
	{
		return;
	}

	// load balance algorithm.
	uint16_t min_workload = 0;
	auto min_workload_server = m_address_set.begin();
	for (auto it = m_address_set.begin(); it != m_address_set.end(); ++it)
	{
		if (it->m_workload > min_workload)
		{
			min_workload = it->m_workload;
			min_workload_server = it;
		}
	}
	if (min_workload_server != m_address_set.end())
	{
		++min_workload_server->m_workload;
		m_address_cur = *min_workload_server;
		m_peer->async_connect(m_address_cur.m_address);
	}
}



////////////////////////////////////////////////////////////////////////////////
ECO_OBJECT_IMPL(TcpClient);
ECO_PROPERTY_STR_IMPL(TcpClient, service_name);

////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::async_connect()
{
	eco::Mutex::ScopeLock lock(m_mutex);

	// verify data.
	if (m_prot_head.get() == nullptr)
		EcoThrow(e_client_no_protocol_head) << "client protocol head is null.";
	if (m_protocol.get() == nullptr)
		EcoThrow(e_client_no_protocol) << "client protocol is null.";
	if (m_balancer.m_address_set.empty())
		EcoThrow(e_client_no_address) << "client has no server address.";
	if (m_make_session == nullptr)
		EcoThrow(e_client_no_session_data) << "client has no session data.";

	// init tcp client worker. 
	if (!m_init.is_ok())
	{
		// start io server, timer and dispatch server.
		m_worker.run();
		m_dispatcher.run();		
		// start timer.
		m_timer.set_io_service(*(IoService*)m_worker.get_io_service());
		m_timer.register_on_timer(std::bind(
			&Impl::on_timer, this, std::placeholders::_1));
		set_tick_timer();
		m_init.ok();
	}
	m_balancer.load_server();
}


///////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::release()
{
	eco::Mutex::ScopeLock lock(m_mutex);
	if (m_init.is_ok())
	{
		EcoInfo << "tcp client closing.";
		m_timer.cancel();
		m_balancer.m_peer->close();
		m_worker.stop();
		m_dispatcher.stop();
		m_init.none();
		EcoInfo << "tcp client closed.";
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::async_authorize(
	IN Codec& codec,
	IN TcpSession& session,
	IN const uint32_t type,
	IN const MessageCategory category)
{
	// send authority to server for validating session.
	// if the authority is correct, server and client will build a new session.
	SessionDataPack::ptr pack(new SessionDataPack);
	pack->m_session.reset(m_make_session(none_session));
	session.set_host(TcpSessionHost(*m_client));
	session.impl().m_session_wptr = pack->m_session;
	session.impl().user_born();
	pack->m_user_observer = session.impl().m_user;

	// encode the send data.
	eco::Error e;
	MessageMeta meta(codec, none_session,
		type, model_req, category | category_authority);
	// use "&session" when get response, because it represent user.
	meta.set_request_data(&session);
	if (!m_protocol->encode(pack->m_request, meta, *m_prot_head, e))
	{
		EcoError << "tcp client async_authorize: encode data fail."
			<< EcoFmt(e);
		return;
	}
	async_send(&session, pack);
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_connect()
{
	eco::String data;
	eco::Mutex::ScopeLock lock(m_authority_map.mutex());
	auto it = m_authority_map.map().begin();
	for (; it != m_authority_map.map().end(); ++it)
	{
		SessionDataPack& pack = (*it->second);
		data.asign(pack.m_request.c_str(), pack.m_request.size());
		async_send(data);
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_close(IN uint64_t peer_id)
{
	m_session_map.clear();
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_read(IN void* peer, IN eco::String& data)
{
	auto* peer_impl = static_cast<TcpPeer::Impl*>(peer);

	// parse message head.
	eco::Error e;
	MessageHead head;
	if (!m_prot_head->decode(head, data, e))
	{
		EcoNet(EcoError, *peer_impl, "on_read", e);
		return;
	}

	// get related protocol.
	if (m_protocol == nullptr)
	{
		e.id(e_message_unknown) << "tcp client have no protocol: "
			<< head.m_version;
		EcoNet(EcoError, *peer_impl, "on_read", e);
		return;
	}

	// ignore sync request: don't support sync mode.
	if (eco::has(head.m_category, category_sync_mode))
	{
		return;
	}

	// handle heartbeat.
	if (eco::has(head.m_category, category_heartbeat) &&
		m_option.response_heartbeat())
	{
		peer_impl->async_send_heartbeat(*m_prot_head);
		return;
	}

	TcpSessionHost host(*m_client);
	DataContext dc(&host);
	peer_impl->get_data_context(dc, head.m_category, data, 
		*m_protocol, *m_prot_head);
	m_dispatcher.post(dc);
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_timer(IN const eco::Error* e)
{
	if (e != nullptr)
	{
		return;
	}
	m_option.step_tick();
	EcoDebug << "...tcp client on tick........................................";

	// send rhythm heartbeat.
	if (m_option.get_heartbeat_send_tick() > 0 &&
		m_option.tick_count() % m_option.get_heartbeat_send_tick() == 0)
	{
		async_send_heartbeat();
	}

	// #.auto reconnect.
	if (m_option.auto_reconnect_tick() > 0 &&
		m_option.tick_count() % m_option.auto_reconnect_tick() == 0)
	{
		async_connect();
	}

	set_tick_timer();		// set next timer.
}


//##############################################################################
//##############################################################################
////////////////////////////////////////////////////////////////////////////////
DispatchRegistry& TcpClient::dispatcher()
{
	return impl().m_dispatcher;
}

void TcpClient::set_session_data(IN MakeSessionDataFunc make)
{
	impl().m_make_session = make;
}

void TcpClient::release()
{
	impl().release();
}

void TcpClient::set_protocol_head(IN ProtocolHead* heap)
{
	impl().m_prot_head.reset(heap);
}

void TcpClient::set_protocol(IN Protocol* heap)
{
	impl().m_protocol.reset(heap);
}

void TcpClient::async_connect(IN eco::net::AddressSet& service_addr)
{
	impl().async_connect(service_addr);
}

void TcpClient::async_send(IN eco::String& data)
{
	impl().async_send(data);
}

void TcpClient::async_send(
	IN Codec& codec,
	IN const uint32_t session_id,
	IN const uint32_t type,
	IN const MessageModel model,
	IN const MessageCategory category)
{
	impl().async_send(codec, session_id, type, model, category);
}

void TcpClient::async_authorize(
	IN Codec& codec,
	IN TcpSession& session,
	IN const uint32_t type,
	IN const MessageCategory category)
{
	impl().async_authorize(codec, session, type, category);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
