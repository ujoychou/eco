#include "PrecHeader.h"
#include "TcpClient.ipp"
////////////////////////////////////////////////////////////////////////////////
#include "TcpOuter.h"
#include <eco/net/protocol/TcpProtocol2.h>
#include <random>
#include <chrono>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
void LoadBalancer::add_address(IN Address& addr)
{
	m_address_set.push_back(addr);
}
void LoadBalancer::update_address(IN AddressSet& addr)
{
	m_address_set = addr;
	m_address_cur = -1;		// need random again.
}


////////////////////////////////////////////////////////////////////////////////
bool LoadBalancer::connect()
{
	// this client has connect to server.
	if (m_peer->get_state().connected() || m_address_set.empty())
	{
		return false;
	}

	// load balance algorithm: generate random number.
	int size = static_cast<int>(m_address_set.size());
	if (m_address_cur == -1)
	{
		using namespace std::chrono;
		uint32_t seed = (uint32_t)system_clock::now().time_since_epoch().count();
		std::default_random_engine gen(seed);
		std::uniform_int_distribution<int> distrib(0, size - 1);
		m_address_cur = distrib(gen);
	}
	else
	{
		m_address_cur = (m_address_cur + 1) % size;
	}
	m_peer->async_connect(m_address_set.at(m_address_cur));
	return true;
}


////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(TcpClient);
void TcpClient::Impl::async_connect(IN bool reconnect)
{
	eco::Mutex::ScopeLock lock(m_mutex);
	// set default protocol.
	if (m_protocol.protocol_latest() == 0)
	{
		if (!m_option.websocket())
		{
			m_protocol.add_protocol(new TcpProtocol());
			//m_protocol.add_protocol(new TcpProtocol2());
		}
		else
		{
			m_protocol.add_protocol(new WebSocketProtocol(true));
		}
	}

	// verify data.
	eco::Error e;
	if (m_balancer.m_address_set.empty())
	{
		e.id(e_client_no_address) << "client has no server address.";
	}
	if (e)
	{
		ECO_LOGX(info) << logging() << e;
		return;
	}

	// init tcp client worker. 
	if (m_balancer.m_peer == nullptr)
	{
		// peer handler init.
		using namespace std::placeholders;
		m_peer_handler.m_owner = this;
		m_peer_handler.m_option = &m_option;
		m_peer_handler.m_protocol = &m_protocol;
		m_peer_handler.m_websocket_key = websocket_key();
		m_peer_handler.m_on_close = 
			std::bind(&TcpClient::Impl::on_close, this, _1, _2, _3);
		m_peer_handler.m_on_read = 
			std::bind(&TcpClient::Impl::on_read, this, _1, _2, _3);
		m_peer_handler.m_on_send = 
			std::bind(&TcpClient::Impl::on_send, this, _1, _2);
		m_peer_handler.m_on_connect = 
			std::bind(&TcpClient::Impl::on_connect, this, _1);

		// start io server, timer and dispatch server.
		std::string name("worker-");
		name += m_option.get_name();
		m_worker.run(name.c_str());
		m_dispatch.run(m_option.get_name());
		m_dispatch.set_capacity(m_option.get_context_capacity());
		// create peer.
		m_balancer.m_peer = TcpPeer::make(
			m_worker.get_io_worker(), 0, &m_peer_handler);
		if (m_make_connection)
		{
			m_balancer.m_peer->impl().make_connection_data(
				m_make_connection, nullptr);
		}
		// start timer.
		m_timer.set_io_service(*m_worker.get_io_service());
		m_timer.register_on_timer(
			std::bind(&Impl::on_timer, this, std::placeholders::_1));
		set_tick_timer();
	}
	else if (reconnect)		// reset connection state.
	{
		m_balancer.m_peer->impl().state().set_closed();
	}
	
	if (m_balancer.connect())
	{
		ECO_LOGX(info) << logging();
	}
}


///////////////////////////////////////////////////////////////////////////////
inline String TcpClient::Impl::logging()
{
	eco::StreamX log;
	log.buffer().reserve(512);
	log << "\n+[tcp client " << m_option.get_name() << "]\n";

	// log address set and cur address.
	for (int i = 0; i < (int)m_balancer.m_address_set.size(); ++i)
	{
		auto& it = m_balancer.m_address_set.at(i);
		if (m_balancer.m_address_cur == i)
			log < "@[addr]" <= it < '\n';
		else
			log < "-[addr]" <= it < '\n';
	}

	// log tcp client option.
	auto lost = m_option.get_heartbeat_recv_tick() * m_option.get_tick_time();
	auto send = m_option.get_heartbeat_send_tick() * m_option.get_tick_time();
	auto conn = m_option.get_auto_reconnect_tick() * m_option.get_tick_time();
	log << "-[this] " << get_ip() << '\n';
	log << "-[mode] no_delay" << eco::group(eco::yn(m_option.no_delay()))
		<< ", websocket" << eco::group(eco::yn(m_option.websocket()))
		<< ", send_buff" << eco::group(m_option.send_buffer_size())
		<< ", recv_buff" << eco::group(m_option.recv_buffer_size())
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
	if (peer())
	{
		auto id = peer()->get_id();
		m_timer.close();
		m_session_map.clear();
		m_balancer.release();
		m_worker.stop();
		m_dispatch.stop();
		ECO_INFO << NetLog(id, __func__);
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
	conn.set_peer(peer()->impl().m_peer_observer);
	conn.set_protocol(*m_protocol.protocol_latest());
	// set pack data.
	SessionDataPack::ptr pack(new SessionDataPack);
	pack->m_session.reset(m_make_session(none_session, sess.impl().m_conn));
	sess.impl().m_session_wptr = pack->m_session;
	sess.impl().m_owner.set(this, false);
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
		ECO_THROW(e_client_session_not_opened) << "this session is not opened.";
	}

	// encode the send data.
	eco::Error e;
	meta.m_session_id = 0;
	meta.m_category |= (category_authority | category_session);
	// use "&session" when get response, because it represent user.
	pack->m_request_data = meta.m_request_data;
	meta.option(session_key);
	if (!m_protocol.protocol_latest()->encode(
		pack->m_request, pack->m_request_start, meta, e))
	{
		ECO_INFO << "tcp client async auth: encode data fail." << e;
		return;
	}
	send(pack);
}


////////////////////////////////////////////////////////////////////////////////
eco::Result TcpClient::Impl::request(
	IN MessageMeta& req,
	IN Codec* err_codec,
	IN Codec* rsp_codec,
	IN std::vector<void*>* rsp_set)
{
	// async manager.
	uint32_t req_id = ++m_request_id;
	req.set_request_data(req_id);
	eco::add(req.m_category, category_sync);
	auto sync = post_sync(req_id, err_codec, rsp_codec, rsp_set);

	// send message.
	send(req);
	auto result = sync->m_monitor.timed_wait(m_timeout_millsec);
	auto ec = sync->m_error_id > 0 ? sync->m_error_id : result;
	erase_sync(req_id);
	eco::thread::release(sync);
	return ec;
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::async(IN MessageMeta& req, IN ResponseFunc& rsp_func)
{
	// async send message.
	uint32_t req_id = ++m_request_id;
	req.set_request_data(req_id);
	eco::add(req.m_category, category_sync);
	post_async(req_id, rsp_func);
	send(req);
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_connect(IN const eco::Error* e)
{
	eco::Mutex::ScopeLock lock(m_mutex);
	if (e == nullptr)
	{
		peer()->impl().init_option(m_option);

		// auto clear data state.
		peer()->data_state().none();

		// reconnect to server: session authority.
		auto it = m_authority_map.begin();
		for (; it != m_authority_map.end(); ++it)
		{
			eco::String data;
			SessionDataPack& pack = (*it->second);
			data.assign(pack.m_request.c_str(), pack.m_request.size());
			send(data, pack.m_request_start);
		}
		ECO_INFO << NetLog(peer()->get_id(), __func__)
			<= m_option.get_name();
	}
	else
	{
		ECO_ERROR << NetLog(peer()->get_id(), __func__)
			<= m_option.get_name() <= *e;
	}

	// notify on connect.
	if (m_on_open)
	{
		m_on_open(e);
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_close(
	ConnectionId peer_id, const Error& e, bool erase_peer)
{
	m_session_map.clear();
	ECO_INFO << NetLog(peer_id, __func__);

	// notify on close.
	if (m_on_close)
	{
		m_on_close(e);
	}
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
	if (m_option.is_time_to_heartbeat_send_tick())
	{
		send_heartbeat();
	}

	// #.auto reconnect.
	if (m_option.is_time_to_reconnect_tick())
	{
		async_connect(false);
	}

	// #.clean dead peer.
	if (m_option.is_time_to_heartbeat_recv_tick())
	{
		check_peer_live();
	}

	set_tick_timer();		// set next timer.
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_read(
	IN void* impl, IN MessageHead& head, IN eco::String& data)
{
	auto* peer = static_cast<TcpPeer::Impl*>(impl);

	// #.send heartbeat.
	if (is_heartbeat(head.m_category))
	{
		peer->state().set_peer_live(true);
		if (m_option.response_heartbeat())
		{
			peer->send_heartbeat(*head.m_protocol);
		}
		ECO_DEBUG << NetLog(peer->get_id(), __func__) <= "heartbeat";
		return;
	}

	// #.dispatch data context.
	DataContext dc;
	peer->get_data_context(dc, head, data);
	m_dispatch.queue().post(dc);
}
void TcpClient::Impl::on_send(IN void* peer, IN uint32_t size)
{}


//##############################################################################
//##############################################################################
ECO_PROPERTY_VAL_IMPL(TcpClient, TcpClientOption, option);
////////////////////////////////////////////////////////////////////////////////
void TcpClient::register_handler(IN uint64_t id, IN HandlerFunc hf)
{
	impl().m_dispatch.register_handler(id, std::move(hf));
}
void TcpClient::register_default_handler(IN HandlerFunc hf)
{
	impl().m_dispatch.register_default_handler(std::move(hf));
}
void TcpClient::set_event(ClientOpenFunc on_open, ClientCloseFunc on_close)
{
	impl().m_on_open = on_open;
	impl().m_on_close = on_close;
}
void TcpClient::set_recv_event(
	OnRecvDataFunc on_recv, OnDecodeHeadFunc on_decode)
{
	impl().m_peer_handler.m_on_decode_head = on_decode;
	impl().m_dispatch.message_handler().set_event(on_recv);
}
void TcpClient::set_timeout(IN const uint32_t millsec)
{
	impl().m_timeout_millsec = millsec;
}
void TcpClient::set_connection_data(IN MakeConnectionDataFunc make)
{
	impl().m_make_connection = make;
}


////////////////////////////////////////////////////////////////////////////////
ConnectionData* TcpClient::data()
{
	return impl().m_balancer.m_peer->data();
}
eco::atomic::State& TcpClient::data_state()
{
	return impl().m_balancer.m_peer->data_state();
}
const TcpState& TcpClient::get_state() const
{
	return impl().m_balancer.m_peer->get_state();
}
void TcpClient::set_session_data(IN MakeSessionDataFunc make)
{
	impl().m_make_session = make;
}
ConnectionId TcpClient::get_id()
{
	return impl().peer()->get_id();
}
void TcpClient::close()
{
	impl().close();
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::add_protocol(IN Protocol* heap)
{
	impl().m_protocol.add_protocol(heap);
}
Protocol* TcpClient::protocol(IN int version) const
{
	return impl().m_protocol.protocol(version);
}
Protocol* TcpClient::protocol_latest() const
{
	return impl().m_protocol.protocol_latest();
}
void TcpClient::add_address(IN Address& addr)
{
	impl().add_address(addr);
}
void TcpClient::set_address(IN AddressSet& addr)
{
	impl().update_address(addr);
	impl().m_option.set_name(addr.get_name());
}
void TcpClient::async_connect(IN bool reconnect)
{
	impl().async_connect(reconnect);
}
void TcpClient::async_connect(IN AddressSet& addr, IN bool reconnect)
{
	impl().async_connect(addr, reconnect);
}
void TcpClient::connect(IN uint32_t millsec)
{
	impl().async_connect(false);
	eco::thread::time_wait(std::bind(&TcpState::connected,
		&impl().peer()->get_state()), millsec);
}
void TcpClient::connect(IN AddressSet& addr, IN uint32_t millsec)
{
	impl().async_connect(addr, false);
	eco::thread::time_wait(std::bind(&TcpState::connected,
		&impl().peer()->get_state()), millsec);
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::send(IN eco::String& data, IN const uint32_t start)
{
	impl().send(data, start);
}
void TcpClient::send(IN MessageMeta& meta)
{
	impl().send(meta);
}
TcpSession TcpClient::open_session()
{
	return impl().open_session();
}
void TcpClient::authorize(IN TcpSession& session, IN MessageMeta& meta)
{
	TcpSessionOuter outer(session);
	impl().async_auth(outer.impl(), meta);
}
eco::Result TcpClient::request(
	IN MessageMeta& req,
	IN Codec* err_codec,
	IN Codec* rsp_codec,
	IN std::vector<void*>* rsp_set)
{
	return impl().request(req, err_codec, rsp_codec, rsp_set);
}
void TcpClient::async(IN MessageMeta& req, IN ResponseFunc& rsp_func)
{
	impl().async(req, rsp_func);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
