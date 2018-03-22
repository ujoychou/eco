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
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
bool LoadBalancer::connect()
{
	// this client has connect to server.
	if (m_peer->get_state().connected() || m_address_set.empty())
	{
		return false;
	}

	// load balance algorithm.
	auto min_workload_server = m_address_set.begin();
	uint16_t min_workload = min_workload_server->m_workload;
	for (auto it = m_address_set.begin(); it != m_address_set.end(); ++it)
	{
		if (it->m_workload < min_workload)
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
	return true;
}



////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(TcpClient);
///////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::init()
{
	// set protocol.
	if (m_option.websocket())
	{
		set_protocol_head(new WebSocketProtocolHeadEx());
		set_protocol(new WebSocketProtocol(true));	// mask frame.
	}
	else if (m_protocol == nullptr)
	{
		set_protocol_head(new TcpProtocolHead());
		set_protocol(new TcpProtocol());
	}
}


///////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::async_connect()
{
	eco::Mutex::ScopeLock lock(m_mutex);

	// verify data.
	eco::Error e;
	if (m_prot_head.get() == nullptr)
	{
		e.id(e_client_no_protocol_head) << "client protocol head is null.";
	}
	else if (m_protocol.get() == nullptr)
	{
		e.id(e_client_no_protocol_head) << "client protocol is null.";
	}
	else if (m_balancer.m_address_set.empty())
	{
		e.id(e_client_no_address) << "client has no server address.";
	}
	if (e)
	{
		EcoLogStr(info, 512) << logging() << e;
		return;
	}

	// init tcp client worker. 
	if (!m_init.is_ok())
	{
		// start io server, timer and dispatch server.
		m_worker.run();
		m_dispatcher.run();	
		// create peer.
		m_balancer.m_peer = TcpPeer::make(m_worker.get_io_service(), this);
		// start timer.
		m_timer.set_io_service(*(IoService*)m_worker.get_io_service());
		m_timer.register_on_timer(
			std::bind(&Impl::on_timer, this, std::placeholders::_1));
		set_tick_timer();
		m_init.ok();
	}
	
	if (m_balancer.connect())
	{
		EcoLogStr(info, 512) << logging();
	}
}


///////////////////////////////////////////////////////////////////////////////
inline String TcpClient::Impl::logging()
{
	Stream log;
	log.buffer().reserve(512);
	log << "\n+[tcp client " << m_option.get_service_name() << "]\n";

	// log address set and cur address.
	auto addr_set = m_balancer.m_address_set;
	for (auto it = addr_set.begin(); it != addr_set.end(); ++it)
	{
		if (m_balancer.m_address_cur == *it)
			log < "@[addr]" <= it->m_address < '\n';
		else
			log < "-[addr]" <= it->m_address < '\n';
	}

	// log tcp client option.
	auto lost = m_option.get_heartbeat_recv_tick() * m_option.get_tick_time();
	auto send = m_option.get_heartbeat_send_tick() * m_option.get_tick_time();
	auto conn = m_option.get_auto_reconnect_tick() * m_option.get_tick_time();
	log << "-[this] " << get_ip() << '\n';
	log << "-[mode] io delay" << eco::group(eco::yn(m_option.no_delay()))
		<< ", websocket" << eco::group(eco::yn(m_option.websocket()))
		<< ", sessions\n"
		<< "-[tick] " << m_option.get_tick_time() << 's'
		<< ", lost server " << lost << 's'
		<< ", heartbeat " << send << 's'
		<< ", reconnect " << conn << "s\n";
	return std::move(log.buffer());
}


///////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::close()
{
	eco::Mutex::ScopeLock lock(m_mutex);
	if (m_init.is_ok())
	{
		auto id = peer().get_id();
		m_timer.cancel();
		m_balancer.release();
		m_worker.stop();
		m_dispatcher.stop();
		m_init.none();
		EcoInfo << NetLog(id, ECO_FUNC);
	}
}


////////////////////////////////////////////////////////////////////////////////
TcpSession TcpClient::Impl::open_session()
{
	// create new session.
	TcpSession session;
	TcpSessionOuter sess(session);
	// set connection data.
	TcpConnectionOuter conn(sess.impl().m_conn);
	conn.set_peer(peer().impl().m_peer_observer);
	conn.set_protocol(*m_protocol);
	// set pack data.
	SessionDataPack::ptr pack(new SessionDataPack);
	pack->m_session.reset(m_make_session(none_session, sess.impl().m_conn));
	sess.impl().m_session_wptr = pack->m_session;
	sess.impl().m_owner.set(*(TcpClientImpl*)this);
	sess.make_client_session();
	pack->m_user_observer = sess.impl().m_user;
	// save pack.
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
	meta.m_session_id = 0;
	meta.m_category |= (category_authority | category_session);
	// use "&session" when get response, because it represent user.
	pack->m_request_data = meta.m_request_data;
	meta.set_request_data(session_key);
	if (!m_protocol->encode(pack->m_request, pack->m_request_start, meta, e))
	{
		EcoError << "tcp client async auth: encode data fail." << e;
		return;
	}
	async_send(pack);
}


////////////////////////////////////////////////////////////////////////////////
eco::Result TcpClient::Impl::request(IN MessageMeta& req, IN Codec& rsp)
{
	// async manager.
	uint32_t req_id = ++m_request_id;
	req.set_request_data(req_id);
	eco::add(req.m_category, category_sync);
	auto async = post_async(req_id, rsp);

	// send message.
	async_send(req);
	auto result = async->m_monitor.timed_wait(m_timeout_millsec);
	if (result != eco::ok)
	{
		pop_async(req_id);
	}
	return result;
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_connect()
{
	eco::Mutex::ScopeLock lock(m_mutex);
	// set peer option: no delay.
	peer().set_option(m_option.no_delay());

	// reconnect to server: session authority.
	for (auto it = m_authority_map.begin(); it != m_authority_map.end(); ++it)
	{
		eco::String data;
		SessionDataPack& pack = (*it->second);
		data.asign(pack.m_request.c_str(), pack.m_request.size());
		async_send(data, pack.m_request_start);
	}
	EcoInfo << NetLog(peer().get_id(), ECO_FUNC);

	// notify on connect.
	if (m_on_connect)
	{
		m_on_connect();
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_close(IN uint64_t peer_id)
{
	m_session_map.clear();
	EcoInfo << NetLog(peer_id, ECO_FUNC);

	// notify on close.
	if (m_on_close)
	{
		m_on_close();
	}
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
		EcoError << NetLog(peer_impl->get_id(), ECO_FUNC) <= e;
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
ECO_PROPERTY_VAL_IMPL(TcpClient, TcpClientOption, option);
////////////////////////////////////////////////////////////////////////////////
DispatchRegistry& TcpClient::dispatcher()
{
	return impl().m_dispatcher;
}

void TcpClient::set_event(IN OnConnectFunc on_connect, IN OnCloseFunc on_close)
{
	impl().m_on_close = on_close;
	impl().m_on_connect = on_connect;
}

void TcpClient::set_session_data(IN MakeSessionDataFunc make)
{
	impl().m_make_session = make;
}

ConnectionId TcpClient::get_id()
{
	return impl().peer().get_id();
}

void TcpClient::close()
{
	impl().close();
}

void TcpClient::set_protocol_head(IN ProtocolHead* heap)
{
	impl().m_prot_head.reset(heap);
}

void TcpClient::set_protocol(IN Protocol* heap)
{
	impl().m_protocol.reset(heap);
}

void TcpClient::set_address(IN eco::net::AddressSet& addr)
{
	impl().update_address(addr);
	impl().m_option.set_service_name(addr.get_name());
}

void TcpClient::async_connect()
{
	impl().init();
	impl().async_connect();
}

void TcpClient::async_connect(IN eco::net::AddressSet& addr)
{
	impl().init();
	impl().async_connect(addr);
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

eco::Result TcpClient::request(IN MessageMeta& req, IN Codec& rsp)
{
	return impl().request(req, rsp);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
