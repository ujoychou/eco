#include "PrecHeader.h"
#include "TcpClient.ipp"
////////////////////////////////////////////////////////////////////////////////
#include "TcpOuter.h"


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
	if (m_peer->get_state().connected())
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
TcpSession TcpClient::Impl::open_session()
{
	// create new session.
	TcpSession session;
	SessionDataPack::ptr pack(new SessionDataPack);
	pack->m_session.reset(m_make_session(none_session));
	TcpSessionOuter sess(session);
	TcpConnectionOuter conn(sess.impl().m_conn);
	sess.impl().m_owner.set(*(TcpClientImpl*)this);
	sess.impl().m_session_wptr = pack->m_session;
	sess.impl().m_peer = m_balancer.m_peer.get();
	sess.make_client_session();
	conn.set_peer(sess.impl().m_peer->impl().m_peer_observer);
	conn.set_protocol(*m_protocol);
	pack->m_user_observer = sess.impl().m_user;
	
	// add session.
	void* session_key = sess.impl().m_user.get();
	set_authority(session_key, pack);
	return session;
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::async_auth(IN TcpSessionImpl& sess, IN MessageMeta& meta)
{
	// send authority to server for validating session.
	// if the authority is correct, server will build a new session.
	void* session_key = sess.m_user.get();
	SessionDataPack::ptr pack = find_authority(session_key);
	if (pack == nullptr)
	{
		EcoThrow(e_client_session_not_opened) << "this session is not opened.";
	}

	// encode the send data.
	eco::Error e;
	meta.m_category |= category_authority;
	// use "&session" when get response, because it represent user.
	pack->m_request_data = meta.m_request_data;
	meta.set_request_data(session_key);
	if (!m_protocol->encode(pack->m_request, pack->m_request_start, meta, e))
	{
		EcoError << "tcp client async auth: encode data fail." << EcoFmt(e);
		return;
	}
	async_send(pack);
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_connect()
{
	eco::Mutex::ScopeLock lock(m_mutex);
	for (auto it = m_authority_map.begin(); it != m_authority_map.end(); ++it)
	{
		eco::String data;
		SessionDataPack& pack = (*it->second);
		data.asign(pack.m_request.c_str(), pack.m_request.size());
		async_send(data, pack.m_request_start);
	}
	m_balancer.m_peer->impl().make_connection_data(
		m_make_connection, m_protocol.get());
	EcoNetLog(EcoDebug, peer()) << "connected.";
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

	// ignore sync request: don't support sync mode.
	if (eco::has(head.m_category, category_sync_mode))
	{
		return;
	}

	// client have no "io heartbeat" option.
	TcpSessionOwner owner(*(TcpClientImpl*)this);
	DataContext dc(&owner);
	peer_impl->get_data_context(dc, head.m_category, data, m_protocol.get());
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
		//async_send_heartbeat();
	}

	// #.auto reconnect.
	if (m_option.auto_reconnect_tick() > 0 &&
		m_option.tick_count() % m_option.auto_reconnect_tick() == 0)
	{
		//async_connect();
	}

	set_tick_timer();		// set next timer.
}


//##############################################################################
//##############################################################################
ECO_PROPERTY_VAL_IMPL(TcpClient, TcpClientOption, option);
////////////////////////////////////////////////////////////////////////////////
DispatchRegistry& TcpClient::dispatcher()
{
	return impl().m_dispatcher;
}

void TcpClient::set_connection_data(IN MakeConnectionDataFunc make)
{
	impl().m_make_connection = make;
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

void TcpClient::async_send(IN eco::String& data, IN const uint32_t start)
{
	impl().async_send(data, start);
}

void TcpClient::async_send(IN MessageMeta& meta)
{
	impl().async_send(meta);
}

TcpSession TcpClient::open_session()
{
	return impl().open_session();
}

void TcpClient::async_auth(IN TcpSession& session, IN MessageMeta& meta)
{
	TcpSessionOuter outer(session);
	impl().async_auth(outer.impl(), meta);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
