#include "PrecHeader.h"
#include "TcpClient.ipp"
////////////////////////////////////////////////////////////////////////////////


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
ECO_OBJECT_IMPL(TcpClient);
ECO_PROPERTY_STR_IMPL(TcpClient, service_name);
////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::async_connect(
	IN const char* service_name_,
	IN eco::net::AddressSet& service_addr)
{
	m_service_name = service_name_;

	// reset flag.
	auto it = m_address_set.begin();
	for (; it != m_address_set.end(); ++it)
	{
		it->m_flag = 0;
	}

	// flag new, old, delete.
	for (auto it = service_addr.begin(); it != service_addr.end(); ++it)
	{
		auto it_find = std::find(
			m_address_set.begin(), m_address_set.end(), *it);
		if (it_find == m_address_set.end())
		{
			AddressLoad load(*it);
			load.m_flag = 1;
			m_address_set.push_back(load);
		}
		else
		{
			it_find->m_flag = 2;
		}
	}

	// delete address and peer.
	for (auto it = m_address_set.begin(); it != m_address_set.end();)
	{
		if (it->m_flag == 0)
		{
			it = m_address_set.erase(it);
			if (m_address_cur == &*it)
			{
				m_address_cur = nullptr;
				m_peer->close();
			}
		}
		else
		{
			++it;
		}
	}

	// reconnect to new address if cur address is removed.
	async_connect();
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::async_connect()
{
	if (m_peer->get_state().is_connected())
	{
		return;
	}

	// check protocol.
	if (m_prot_head.get() == nullptr)
	{
		EcoThrow(e_consumer_protocol_head_null)
			<< "consumer protocol head is null.";
	}
	if (m_protocol.get() == nullptr)
	{
		EcoThrow(e_consumer_protocol_null)
			<< "consumer protocol is null.";
	}

	if (!m_state.is_ok())
	{
		// start io server and message server.
		m_worker.run();
		m_dispatcher.run();		

		// start client timer.
		m_timer.set_io_service(*(IoService*)m_worker.get_io_service());
		m_timer.register_on_timer(std::bind(
			&Impl::on_timer, this, std::placeholders::_1));
		set_tick_timer();

		m_state.ok();
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
	if (min_workload_server == m_address_set.end())
	{
		return;
	}
	m_peer->async_connect(min_workload_server->m_address);
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_connect()
{
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
		e << " tcp client decode head fail.";
		EcoNet(EcoError, *m_peer, "on_read", e);
		return;
	}

	// get related protocol.
	if (m_protocol == nullptr)
	{
		e.id(e_message_unknown) << "tcp client have no protocol: "
			<< head.m_version;
		EcoNet(EcoError, *m_peer, "on_read", e);
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
inline void TcpClient::Impl::set_tick_timer()
{
	// there is no need a tick timer.
	if (!m_option.has_tick_timer())
	{
		return;
	}

	// set a tick timer to do some work in regular intervals.
	uint32_t tick_secs = m_option.get_tick_time();
	m_timer.set_timer(tick_secs);
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_timer(IN const eco::Error* e)
{
	if (e != nullptr)
	{
		return;
	}

	set_tick_timer();
}


//##############################################################################
//##############################################################################
////////////////////////////////////////////////////////////////////////////////
DispatchRegistry& TcpClient::dispatcher()
{
	return impl().m_dispatcher;
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::async_init_service(
	IN const char* service_name_,
	IN eco::net::AddressSet& service_addr)
{
	impl().async_connect(service_name_, service_addr);
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::async_send(
	IN eco::net::Codec& codec,
	IN const uint32_t session_id,
	IN const uint32_t msg_type,
	IN const MessageModel model,
	IN const MessageCategory category)
{
	MessageMeta meta(codec, session_id, msg_type, model);
	impl().m_peer->async_send(meta, *impl().m_protocol, *impl().m_prot_head);
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::async_authorize(
	IN Codec& codec,
	IN TcpSession& session,
	IN const uint32_t type)
{
	// send authority to server for validating session.
	// if the authority is correct, server and client will build a new session.
	SessionDataPack::ptr pack(new SessionDataPack);
	pack->m_session.reset(impl().m_make_session(none_session));
	session.set_host(TcpSessionHost(*this));
	session.impl().m_session_wptr = pack->m_session;
	session.impl().user_born();
	pack->m_user_observer = session.impl().m_user;
	
	// encode the send data.
	eco::Error e;
	eco::String data;
	MessageMeta meta(codec, none_session, type, model_req);
	meta.set_request_data(&session);
	if (!impl().m_protocol->encode(data, meta, *impl().m_prot_head, e))
	{
		EcoNet(EcoError, (*impl().m_peer), "async_authorize", e);
		return ;
	}
	// save authority data bytes.
	pack->m_request.asign(data.c_str(), data.size());
	impl().m_authority_map.set(&session, pack);
	impl().m_peer->async_send(data);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
