#include "Pch.h"
#include "TcpClient.ipp"
////////////////////////////////////////////////////////////////////////////////
#include "TcpOuter.h"
#include <eco/App.h>
#include <eco/net/protocol/TcpProtocol2.h>
#include <random>



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
bool LoadBalancer::connect(uint16_t balanc)
{
	// this client has connect to server.
	if (m_peer->state().connected() || m_address_set.empty())
	{
		return false;
	}

	// load balance algorithm: generate random number.
	int size = static_cast<int>(m_address_set.size());

	// connect to server order defined in config xml.
	if (balanc == TcpClientOption::balance_order)
	{
		// first time connect server.
		if (m_address_cur == -1)
			m_address_cur = 0;
		// when server closed, client will reconnect it 3 times.
		else if (m_address_cur / size >= 3)		// try next server.
			m_address_cur = ((m_address_cur % size) + 1) % size;
		m_address_cur += size;	// count 1 times.
	}
	// random connect to server.
	else
	{
		if (m_address_cur == -1)
		{
			uint32_t seed = (uint32_t)std_chrono::system_clock::now()
				.time_since_epoch().count();
#ifdef ECO_VC100
			std::default_random_engine gen([=](){return seed;});
#else
			std::default_random_engine gen(seed);
#endif
			std::uniform_int_distribution<int> distrib(0, size - 1);
			m_address_cur = distrib(gen);
		}
		else
		{
			m_address_cur = (m_address_cur + 1) % size;
		}
	}
	m_peer->async_connect(m_address_set.at(position()));
	return true;
}


////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(TcpClient);
void TcpClient::Impl::async_connect()
{
	eco::Mutex::ScopeLock lock(m_mutex);
	// set default protocol.
	if (m_protocol.protocol_latest() == 0)
	{
		if (!m_option.websocket())
		{
			m_protocol.add_protocol(new TcpProtocol());
			m_protocol.add_protocol(new TcpProtocol2());
		}
		else
		{
			m_protocol.add_protocol(new WebSocketProtocol(true));
		}
	}
	peer().set_protocol(m_protocol.protocol_latest());

	// verify data.
	if (m_balancer.m_address_set.empty())
	{
		ECO_THIS_ERROR(e_client_no_address)
			<< "client has no server address.";
		ECO_LOG(info) << logging() << eco::this_thread::error();
		return;
	}

	// init tcp client worker. 
	if (m_peer_handler.m_owner == nullptr)
	{
		// peer handler init.
		using namespace std::placeholders;
		m_peer_handler.m_owner = this;
		m_peer_handler.m_option = &m_option;
		m_peer_handler.m_protocol = &m_protocol;
		m_peer_handler.m_websocket_key = websocket_key();
		m_peer_handler.m_on_close = 
			std::bind(&TcpClient::Impl::on_close,
			this, std::placeholders::_1);
		m_peer_handler.m_on_read = 
			std::bind(&TcpClient::Impl::on_read,
			this, std::placeholders::_1,
			std::placeholders::_2, std::placeholders::_3);
		m_peer_handler.m_on_send = 
			std::bind(&TcpClient::Impl::on_send, this,
			std::placeholders::_1, std::placeholders::_2);
		m_peer_handler.m_on_connect = 
			std::bind(&TcpClient::Impl::on_connect, 
			this, std::placeholders::_1);
		
		// start io server, timer and dispatch server.
		std::string name("worker-");
		name += m_option.name();
		m_worker.run(name.c_str());
		m_dispatch.run(m_option.name());
		m_dispatch.set_capacity(m_option.context_capacity());
	}
	
	if (m_balancer.connect(m_option.balance()))
	{
		ECO_LOG(info) << logging();
	}
}
void TcpClient::Impl::connect(uint32_t millsec)
{
	async_connect();
	eco::thread::time_wait(std::bind(
		&TcpState::connected, &peer().state()), millsec);
}
void TcpClient::Impl::connect(eco::net::AddressSet& addr, uint32_t millsec)
{
	update_address(addr);
	connect(millsec);
}


///////////////////////////////////////////////////////////////////////////////
inline String TcpClient::Impl::logging()
{
	eco::String log(512, true);
	log << "\n+[[tcp client " << m_option.name() << "]]\n";

	// log address set and cur address.
	int siz = (int)m_balancer.m_address_set.size();
	for (int i = 0; i < siz; ++i)
	{
		auto& it = m_balancer.m_address_set.at(i);
		if (m_balancer.position() == i)
			log < "@[addr]" <= it < '\n';
		else
			log < "-[addr]" <= it < '\n';
	}

	// log tcp client option.
	auto lost = m_option.heartbeat_recv_sec();
	auto send = m_option.heartbeat_send_sec();
	auto conn = m_option.auto_reconnect_sec();
	log << "-[this] " << get_ip() << '\n';
	log << "-[mode] no_delay" << eco::group(eco::yn(m_option.no_delay()))
		<< ", websocket" << eco::group(eco::yn(m_option.websocket()))
		<< ", send_buff" << eco::group(m_option.send_buffer_size())
		<< ", recv_buff" << eco::group(m_option.recv_buffer_size())
		<< ", sessions\n"
		<< ", lost server " << lost << 's'
		<< ", heartbeat " << send << 's'
		<< ", suspend " << eco::yn(m_option.suspend())
		<< ", reconnect " << conn << "s\n";
	return std::move(log);
}


///////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::release()
{
	eco::Mutex::ScopeLock lock(m_mutex);
	if (m_balancer.m_peer)
	{
		auto id = peer().id();
		m_session_map.clear();
		m_balancer.release();
		m_worker.stop();
		m_dispatch.stop();
		ECO_INFO << NetLog(id, __func__);
	}
}
void TcpClient::Impl::close()
{
	eco::Mutex::ScopeLock lock(m_mutex);
	m_manual_close = true;
	ECO_THIS_ERROR(e_peer_client_close);
	peer().close_and_notify();
}


////////////////////////////////////////////////////////////////////////////////
TcpSession TcpClient::Impl::open_session()
{
	// create new session.
	TcpSession session;
	TcpSessionOuter sess(session);
	// set connection data.
	TcpConnectionOuter conn(sess.impl().m_conn);
	conn.set_peer(peer().m_peer_observer);
	// set pack data.
	SessionDataPack::ptr pack(new SessionDataPack);
	pack->m_session.reset(m_make_session(none_session, sess.impl().m_conn));
	sess.impl().m_session_wptr = pack->m_session;
	sess.impl().m_owner.set(this, false);
	sess.make_client_session();
	pack->m_user_observer = sess.impl().m_user;
	// save pack.
	void* session_key = sess.impl().m_user.get();
	//set_authority(session_key, pack);
	return session;
}


////////////////////////////////////////////////////////////////////////////////
eco::Result TcpClient::Impl::request(
	IN MessageMeta& req,
	IN Codec& err_codec,
	IN Codec& rsp_codec,
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
	if (result != eco::timeout)
		result = sync->m_err ? eco::error : eco::ok;
	erase_sync(req_id);
	eco::thread::release(sync);
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
	send(req);
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_connect(bool error)
{
	eco::Mutex::ScopeLock lock(m_mutex);
	if (!error)
	{
		on_connect_ok();
		ECO_KEY << NetLog(peer().id(), __func__) <= m_option.name();
	}
	else
	{
		ECO_ERROR << NetLog(peer().id(), __func__)	<= m_option.name()
			<= eco::this_thread::error();
		auto_connect();		// 1.auto connect when connection open fail.
	}

	// notify on connect.
	if (m_on_open) m_on_open(error);
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_connect_ok()
{
	m_manual_close = false;
	peer().init_option(m_option);
	m_balancer.on_connect(m_option.balance());

	// #.send rhythm heartbeat.
	if (m_option.heartbeat_send_sec() > 0)
	{
		peer().m_timer_send.cancel();
		peer().m_timer_send = eco::Eco::get().timer().run_after(
			[=] { this->on_send_heartbeat(); },
			std_chrono::seconds(m_option.heartbeat_send_sec()), true);
	}

	// #.clean unlive peer.
	if (m_option.heartbeat_recv_sec() > 0)
	{
		peer().m_timer_recv.cancel();
		peer().m_timer_recv = eco::App::get()->timer().run_after(
			[=] { this->on_live_timeout(); },
			std_chrono::seconds(m_option.heartbeat_recv_sec()), false);
	}

	// async load data.
	on_load_data();
	m_timer_load_events.cancel();
	m_timer_load_events = eco::App::get()->timer().run_after(
		[=] { this->on_load_data(); },
		std_chrono::seconds(m_option.get_load_event_sec()), true);
}
void TcpClient::Impl::on_live_timeout()
{
	ECO_FUNC(warn) << m_option.name();
	peer().close_and_notify();
}
void TcpClient::Impl::on_live(bool active)
{
	if (active)
		peer().get_state().set_peer_active(true);
	else
		peer().get_state().set_peer_live(true);

	if (m_option.heartbeat_recv_sec() > 0)
	{
		peer().m_timer_recv.restart();
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_close(IN void* impl)
{
	TcpPeer::Impl* peer = static_cast<TcpPeer::Impl*>(impl);
	m_session_map.clear();

	// reset loading state.
	for (auto it = m_load_events.begin(); it != m_load_events.end(); ++it)
	{
		LoadEvent::ptr& evt = *it;
		evt->m_state.set_ok(false);
	}
	ECO_INFO << NetLog(peer->id(), __func__) < eco::this_thread::error();

	// notify on close.
	if (m_on_close) m_on_close();

	auto_connect();		// 2.auto connect when connection has been closed.
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_load_data()
{
	uint32_t prev_group = 0;
	for (auto it = m_load_events.begin(); it != m_load_events.end(); ++it)
	{
		LoadEvent::ptr& e = *it;
		if (prev_group == 0 || e->m_group != prev_group)
		{
			if (!e->m_state.ok())
			{
				try
				{
					(*e)();
				}
				catch (std::exception& e)
				{
					ECO_LOG(error, m_option.name()) < __func__  <= e.what();
				}
				prev_group = e->m_group;
			}
		}
	}
}
void TcpClient::Impl::on_load_locale(eco::atomic::State& state)
{
	async<ProtobufCodec, eco::proto::Error, eco::proto::Locale>(
		eco::proto_locale_get_req, nullptr,
		[&state](eco::proto::Error& e, eco::net::Context& c) {
		ECO_LOG(error, "load_locale") < e;
	},
		[&state, this](eco::proto::Locale& loc, eco::net::Context& c) {
		eco::App::get()->locale().add_locale(loc, m_option.module_());
		state.set_ok(true);
		ECO_LOG(info, "load_locale");
	});
}


////////////////////////////////////////////////////////////////////////////////
void TcpClient::Impl::on_read(
	IN void* impl, IN MessageHead& head, IN eco::String& data)
{
	auto* peer = static_cast<TcpPeer::Impl*>(impl);

	// #.send heartbeat.
	if (is_heartbeat(head.m_category))
	{
		on_live();
		ECO_FUNC(debug)	< "heartbeat" <= m_option.name();
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
void TcpClient::register_handler(IN int id, IN HandlerFunc&& hf)
{
	impl().m_dispatch.register_handler(id, std::move(hf));
}
void TcpClient::register_default(IN HandlerFunc&& hf)
{
	impl().m_dispatch.register_default(std::move(hf));
}
void TcpClient::set_event(OnConnect&& on_open, OnDisconnect&& on_close)
{
	impl().m_on_open = on_open;
	impl().m_on_close = on_close;
}
void TcpClient::set_recv_event(OnRecvData&& on_recv, OnDecodeHead&& on_decode)
{
	impl().m_dispatch.message_handler().set_event(std::move(on_recv));
	impl().m_peer_handler.m_on_decode_head = on_decode;
}
void TcpClient::set_timeout(uint32_t millsec)
{
	impl().m_timeout_millsec = millsec;
}
void TcpClient::set_connection_data(MakeConnectionData&& make)
{
	impl().peer().make_connection_data(make);
}


////////////////////////////////////////////////////////////////////////////////
uint32_t TcpClient::set_load_event(OnLoadState&& func, bool app_ready_evt)
{
	impl().m_load_events.push_back(
		std::make_shared<LoadEvent>(func, app_ready_evt));
	return uint32_t(impl().m_load_events.size() - 1);
}
uint32_t TcpClient::set_load_event(OnLoadEvent&& func, bool app_ready_evt)
{
	uint32_t group = uint32_t(impl().m_load_events.size() + 1);
	impl().m_load_events.push_back(
		std::make_shared<LoadEvent>(func, group, app_ready_evt));
	return uint32_t(impl().m_load_events.size() - 1);
}
uint32_t TcpClient::add_load_event(OnLoadEvent&& func)
{
	if (impl().m_load_events.empty() ||
		impl().m_load_events.back()->state_mode())
	{
		ECO_THROW("add load event fail: can't find prev load event.");
	}

	// 1.event link node call once.
	auto prev_evt = impl().m_load_events.back();
	auto curr_evt = std::make_shared<LoadEvent>(func,
		prev_evt->m_group, prev_evt->app_ready_event());
	impl().m_load_events.push_back(curr_evt);

	// 2.event link node async call step by step.
	OnLoadEvent link_evt = std::move(prev_evt->m_on_event);
	prev_evt->m_on_event = [=](OnLoadFinish& f) {
		link_evt([=](void) { f(); (*curr_evt)(); });
	};
	return uint32_t(impl().m_load_events.size() - 1);
}
bool TcpClient::ready() const
{
	auto& evts = impl().m_load_events;
	for (auto it = evts.begin(); it != evts.end(); ++it)
	{
		const LoadEvent::ptr& e = *it;
		if (!e->ready())
		{
			ECO_THIS_ERROR(name());
			return false;
		}
	}
	return true;
}
void TcpClient::load_event(uint32_t evt)
{
	LoadEvent::ptr& it = impl().m_load_events[evt];
	it->m_state.set_ok(false);
	(*it)();
}
bool TcpClient::load_event_finished(uint32_t evt) const
{
	return impl().m_load_events[evt]->m_state.ok();
}


////////////////////////////////////////////////////////////////////////////////
ConnectionData* TcpClient::data()
{
	return impl().m_balancer.m_peer->data();
}
const TcpState& TcpClient::state() const
{
	return impl().m_balancer.m_peer->state();
}
void TcpClient::set_session_data(IN MakeSessionData&& make)
{
	impl().m_make_session = make;
}
void TcpClient::close()
{
	impl().close();
}
void TcpClient::release()
{
	impl().release();
}
eco::String TcpClient::ip() const
{
	return impl().peer().ip();
}
uint32_t TcpClient::port() const
{
	return impl().peer().port();
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
	impl().m_option.set_name(addr.name());
}
void TcpClient::async_connect()
{
	impl().async_connect();
}
void TcpClient::async_connect(IN AddressSet& addr)
{
	impl().async_connect(addr);
}
void TcpClient::connect(IN uint32_t millsec)
{
	impl().connect(millsec);
}
void TcpClient::connect(IN AddressSet& addr, IN uint32_t millsec)
{
	impl().connect(addr, millsec);
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
eco::Result TcpClient::request(
	IN MessageMeta& req,
	IN Codec& err_codec,
	IN Codec& rsp_codec,
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
