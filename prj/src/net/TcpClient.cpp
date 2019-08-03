#include "PrecHeader.h"
#include "TcpClient.ipp"
////////////////////////////////////////////////////////////////////////////////
#include "TcpOuter.h"
#include <eco/codec/Zlib.h>
#include <eco/codec/ZlibFlate.h>
#include <eco/codec/ZlibFlateUnk.h>
#include <eco/net/protocol/Check.h>
#include <eco/net/protocol/Crypt.h>
#include <random>
#include <chrono>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
void LoadBalancer::update_address(IN eco::net::AddressSet& addr)
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
		set_protocol(new TcpProtocol(new CheckAdler32(), new CryptFlate()));
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
		ECO_LOGX(info) << logging() << e;
		return;
	}

	// init tcp client worker. 
	if (m_balancer.m_peer == nullptr)
	{
		// start io server, timer and dispatch server.
		m_worker.run();
		m_dispatch.run(m_option.get_name());
		// create peer.
		m_balancer.m_peer = TcpPeer::make(
			m_worker.get_io_service(), nullptr, this);
		m_balancer.m_peer->impl().make_connection_data(
			m_make_connection, nullptr);
		// start timer.
		m_timer.set_io_service(*(IoService*)m_worker.get_io_service());
		m_timer.register_on_timer(
			std::bind(&Impl::on_timer, this, std::placeholders::_1));
		set_tick_timer();
	}
	
	if (m_balancer.connect())
	{
		ECO_LOGX(info) << logging();
	}
}


///////////////////////////////////////////////////////////////////////////////
inline String TcpClient::Impl::logging()
{
	Stream log;
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
	if (m_balancer.m_peer != nullptr)
	{
		auto id = peer().get_id();
		m_timer.cancel();
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
		ECO_THROW(e_client_session_not_opened) << "this session is not opened.";
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
		ECO_INFO << "tcp client async auth: encode data fail." << e;
		return;
	}
	async_send(pack);
}


////////////////////////////////////////////////////////////////////////////////
eco::Result TcpClient::Impl::request(
	IN MessageMeta& req,
	IN Codec& rsp,
	IN std::vector<void*>* rsp_set)
{
	// async manager.
	uint32_t req_id = ++m_request_id;
	req.set_request_data(req_id);
	eco::add(req.m_category, category_sync);
	auto async = post_sync(req_id, rsp, rsp_set);

	// send message.
	async_send(req);
	auto result = async->m_monitor.timed_wait(m_timeout_millsec);
	erase_sync(req_id);
	eco::thread::release(async);
	return result;
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::async(IN MessageMeta& req, IN ResponseFunc& rsp_func)
{
	// async send message.
	uint32_t req_id = ++m_request_id;
	req.set_request_data(req_id);
	eco::add(req.m_category, category_sync);
	post_async(req_id, rsp_func);
	async_send(req);
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_connect(IN const eco::Error* e)
{
	eco::Mutex::ScopeLock lock(m_mutex);
	if (e == nullptr)
	{
		// set peer option: no delay.
		peer().set_option(m_option.no_delay());

		// auto clear data state.
		peer().data_state().none();

		// reconnect to server: session authority.
		auto it = m_authority_map.begin();
		for (; it != m_authority_map.end(); ++it)
		{
			eco::String data;
			SessionDataPack& pack = (*it->second);
			data.asign(pack.m_request.c_str(), pack.m_request.size());
			async_send(data, pack.m_request_start);
		}
		ECO_INFO << NetLog(peer().get_id(), __func__)
			<= m_option.get_name();
	}
	else
	{
		ECO_ERROR << NetLog(peer().get_id(), __func__)
			<= m_option.get_name() <= *e;
	}

	// notify on connect.
	if (m_on_open)
	{
		m_on_open(e);
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_close(IN const ConnectionId peer_id)
{
	m_session_map.clear();
	ECO_INFO << NetLog(peer_id, __func__);

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
		ECO_INFO << NetLog(peer_impl->get_id(), __func__) <= e;
		return;
	}

	// client have no "io heartbeat" option.
	TcpSessionOwner owner(*(TcpClientImpl*)this);
	DataContext dc(&owner);
	peer_impl->get_data_context(dc, head.m_category, data, m_protocol.get());
	m_dispatch.post(dc);
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

	// #.clean dead peer.
	if (m_option.get_heartbeat_recv_tick() > 0 &&
		m_option.tick_count() % m_option.get_heartbeat_recv_tick() == 0)
	{
		if (!peer().impl().check_peer_live())
		{
			ECO_WARN << "heartbeat lost server peer.";
			peer().close_and_notify();
		}
	}

	set_tick_timer();		// set next timer.
}


//##############################################################################
//##############################################################################
ECO_PROPERTY_VAL_IMPL(TcpClient, TcpClientOption, option);
////////////////////////////////////////////////////////////////////////////////
void TcpClient::register_handler(IN uint64_t id, IN HandlerFunc hf)
{
	impl().m_dispatch.register_handler(id, hf);
}

void TcpClient::register_default_handler(IN HandlerFunc hf)
{
	impl().m_dispatch.register_default_handler(hf);
}

void TcpClient::set_event(IN OpenFunc on_open, IN CloseFunc on_close)
{
	impl().m_on_open = on_open;
	impl().m_on_close = on_close;
}

void TcpClient::set_timeout(IN const uint32_t millsec)
{
	impl().m_timeout_millsec = millsec;
}

void TcpClient::set_connection_data(IN MakeConnectionDataFunc make)
{
	impl().m_make_connection = make;
}

ConnectionData* TcpClient::data()
{
	return impl().m_balancer.m_peer->data();
}
eco::atomic::State& TcpClient::data_state()
{
	return impl().m_balancer.m_peer->data_state();
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
	impl().m_option.set_name(addr.get_name());
}

void TcpClient::connect()
{
	impl().init();
	impl().async_connect();
}

void TcpClient::connect(IN eco::net::AddressSet& addr)
{
	impl().init();
	impl().async_connect(addr);
}

void TcpClient::connect_wait(IN uint32_t millsec)
{
	impl().init();
	impl().async_connect();
	eco::thread::time_wait(std::bind(&TcpState::connected,
		&impl().peer().get_state()), millsec);
}

void TcpClient::connect_wait(IN eco::net::AddressSet& addr, IN uint32_t millsec)
{
	impl().init();
	impl().async_connect(addr);
	eco::thread::time_wait(std::bind(&TcpState::connected,
		&impl().peer().get_state()), millsec);
}

void TcpClient::send(IN eco::String& data, IN const uint32_t start)
{
	impl().async_send(data, start);
}

void TcpClient::send(IN MessageMeta& meta)
{
	impl().async_send(meta);
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
	IN MessageMeta& req, IN Codec& rsp,	IN std::vector<void*>* rsp_set)
{
	return impl().request(req, rsp, rsp_set);
}

void TcpClient::async(IN MessageMeta& req, IN ResponseFunc& rsp_func)
{
	impl().async(req, rsp_func);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
