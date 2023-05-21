#ifndef ECO_NET_TCP_CLIENT_IPP
#define ECO_NET_TCP_CLIENT_IPP
#include <eco/net/TcpClient.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/thread/State.h>
#include <eco/thread/Map.h>
#include <eco/thread/Monitor.h>
#include <eco/thread/Timing.h>
#include <eco/net/Worker.h>
#include <net/protocol/WebSocketProtocol.h>
#include "TcpPeer.ipp"
#include "Router.h"


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class LoadBalancer
{
public:
	TcpPeer::ptr	m_peer;
	AddressSet		m_address_set;
	int				m_address_cur;

public:
	inline LoadBalancer() : m_address_cur(-1)
	{}

	inline int position()
	{
		return m_address_cur % m_address_set.size();
	}

	inline void release()
	{
		m_peer->close();
		// release peer before io service stop.
		m_peer.reset();
	}

	inline void on_connect(uint16_t balanc)
	{
		if (balanc == TcpOptionClient::balance_order)
		{
			m_address_cur = position();		// reset reconnect times.
		}
	}

	void add_address(IN Address& addr);
	void update_address(IN AddressSet& addr);

	// connect to server by blance algorithm.
	bool connect(uint16_t balanc);
};


////////////////////////////////////////////////////////////////////////////////
class SessionDataPack
{
	ECO_OBJECT(SessionDataPack);
public:
	SessionData::ptr m_session;
	ClientUserObserver m_user_observer;
	uint64_t m_option_data;

public:
	inline SessionDataPack() : m_option_data(0)
	{}
};


////////////////////////////////////////////////////////////////////////////////
class SyncRequest : public eco::Object<SyncRequest>
{
public:
	inline SyncRequest(
		IN Codec& err_codec,
		IN Codec& rsp_codec,
		IN std::vector<void*>* rsp_set)
		: m_err(false)
		, m_err_codec(err_codec)
		, m_rsp_codec(rsp_codec)
		, m_rsp_set(rsp_set)
	{}

	// if decode "error or response" message fail. return false, else true.
	inline bool decode(const eco::Bytes& msg, bool err)
	{
		// 1.parse reject error object.
		if ((m_err = err))
		{
			if (!m_err_codec.empty() && 
				!m_err_codec.decode(msg.m_data, msg.m_size))
				return false;					// #.decode error msg fail.
		}

		// 2.parse response object or response_set.
		if (msg.m_size > 0 && !m_rsp_codec.empty())
		{
			void* obj = m_rsp_codec.decode(msg.m_data, msg.m_size);
			if (obj == nullptr) return false;	// #.decode rsp msg fail.
			if (m_rsp_set) m_rsp_set->push_back(obj);
		}
		return true;
	}

	uint32_t	m_err;
	Codec&		m_err_codec;
	Codec&		m_rsp_codec;
	std::vector<void*>* m_rsp_set;
	eco::Monitor m_monitor;
};
class AsyncRequest : public eco::Object<AsyncRequest>
{
public:
	inline AsyncRequest(std::function<void(eco::net::Context& c)>& f)
		: m_func(f) {}
	std::function<void(eco::net::Context&)> m_func;
};


////////////////////////////////////////////////////////////////////////////////
class LoadEvent
{
	ECO_OBJECT(LoadEvent);
public:
	inline LoadEvent(OnLoadEvent& evt, uint32_t group, bool app_read_evt)
		: m_group(group), m_on_event(evt)
	{
		m_state.set(app_read_evt, eco::atomic::State::_aa);
	}

	inline LoadEvent(OnLoadState& evt, bool app_read_evt)
		: m_group(0), m_on_state(evt) 
	{
		m_state.set(app_read_evt, eco::atomic::State::_aa);
	}

	inline void operator()(void)
	{
		if (m_on_state) return m_on_state(m_state);
		if (m_on_event) return m_on_event([&](void) { m_state.set_ok(true); });
	}

	inline bool state_mode() const
	{
		return m_on_state != nullptr;
	}

	inline bool ready() const
	{
		return m_state.ok() || !m_state.has(eco::atomic::State::_aa);
	}

	inline bool app_ready_event() const
	{
		return m_state.has(eco::atomic::State::_aa);
	}
	
	uint32_t	m_group;		// event group to implement link event.
	OnLoadEvent	m_on_event;		// callback load event. on_event(on_finish())

	OnLoadState m_on_state;		// state load event. on_event(state)
	eco::atomic::State m_state;	// event finish state.
};


////////////////////////////////////////////////////////////////////////////////
class TcpClient::Impl
{
	ECO_IMPL_INIT(TcpClient);
public:
	LoadBalancer	m_balancer;		// load balance for multi server.
	TcpOptionClient	m_option;		// client option.
	ProtocolFamily  m_protocol;		// client protocol.
	TcpPeerHandler	m_peer_handler;
	uint32_t		m_manual_close;	// manual close means no need auto connect.
	
	// io server, timer and business dispatcher server.
	eco::net::Worker	m_worker;	// io thread.
	Router		m_dispatch;	// dispatcher thread.

	// connection event.
	OnConnect m_on_open;
	OnDisconnect m_on_close;
	std::vector<LoadEvent::ptr> m_load_events;
	Timing::Timer m_timer_load_events;

	// session data management.
	MakeSessionData m_make_session;
	// connected session that has id build by server.
	eco::HashMap<uint64_t, SessionDataPack::ptr> m_session_map;
	mutable std_mutex m_mutex;

	// async management.
	std_atomic_uint32_t m_request_id;
	uint32_t m_timeout_millsec;
	std::unordered_map<uint32_t, SyncRequest::ptr> m_sync_manager;
	std::unordered_map<uint32_t, AsyncRequest::ptr> m_async_manager;

public:
	inline Impl() 
		: m_manual_close(false)
		, m_on_open(nullptr)
		, m_on_close(nullptr)
		, m_make_session(nullptr)
		, m_timeout_millsec(5000)
	{
		// create peer.
		m_balancer.m_peer = TcpPeer::make(
			m_worker.get_io_worker(), 0, &m_peer_handler);
	}

	inline ~Impl()
	{
		release();
	}

	// get current tcp peer connected to server.
	inline TcpPeer::Impl& peer()
	{
		return m_balancer.m_peer->impl();
	}
	inline const TcpPeer::Impl& peer() const
	{
		return m_balancer.m_peer->impl();
	}

	// peer state.
	inline TcpState& get_state()
	{
		return m_balancer.m_peer->impl().get_state();
	}
	inline const TcpState& state() const
	{
		return m_balancer.m_peer->impl().state();
	}

	inline uint32_t set_load_event(OnLoadState func, bool app_ready_evt)
	{
		m_load_events.push_back(
			std::make_shared<LoadEvent>(func, app_ready_evt));
		return uint32_t(m_load_events.size() - 1);
	}

	// open a session with no authority.
	inline TcpSession open_session();

	// find exist session.
	inline SessionDataPack::ptr find_session(IN const SessionId id) const
	{
		SessionDataPack::ptr v;
		m_session_map.find(v, id);
		return v;
	}

	// erase session.
	inline void erase_session(IN const SessionId id)
	{
		SessionDataPack::ptr v = m_session_map.pop(id);
		if (v != nullptr)
		{
			eco::thread::release(v);
		}
	}

	// async management post item.
	inline SyncRequest::ptr post_sync(
		IN uint32_t req_id,
		IN Codec& err_codec,
		IN Codec& rsp_codec,
		IN std::vector<void*>* rsp_set = nullptr)
	{
		std_lock_guard lock(m_mutex);
		auto& ptr = m_sync_manager[req_id];
		if (ptr == nullptr)
			ptr.reset(new SyncRequest(err_codec, rsp_codec, rsp_set));
		return ptr;
	}

	// async management pop item.
	inline SyncRequest::ptr pop_sync(IN uint32_t req_id, IN bool last)
	{
		std_lock_guard lock(m_mutex);
		SyncRequest::ptr sync;
		auto it = m_sync_manager.find(req_id);
		if (it != m_sync_manager.end())
		{
			sync = it->second;
			if (last) m_sync_manager.erase(it);
		}
		return sync;
	}
	inline void erase_sync(IN const uint32_t req_id)
	{
		std_lock_guard lock(m_mutex);
		m_sync_manager.erase(req_id);
	}

	// auto connect to tcpclient, when: 1.on_open; 2.on_close.
	inline void auto_connect()
	{
		if (m_option.get_auto_reconnect_sec() > 0 && !m_manual_close)
		{
			// timer_recv as the timer connect, and set recv timer when peer
			// has connected.
			peer().m_timer_recv.cancel();
			peer().m_timer_recv = eco::Eco::get().timing().run_after([=] {
				if (!get_state().connected() && !m_manual_close)
					this->connect(2000);
			}, std_chrono::seconds(m_option.auto_reconnect_sec()), false);
		}
	}

public:
	// async management post item.
	inline AsyncRequest::ptr post_async(
		IN const uint32_t req_id,
		IN std::function<void(eco::net::Context&)>&& rsp_func)
	{
		std_lock_guard lock(m_mutex);
		auto& ptr = m_async_manager[req_id];
		if (ptr == nullptr)
			ptr.reset(new AsyncRequest(rsp_func));
		return ptr;
	}

	// async management pop item.
	inline AsyncRequest::ptr pop_async(IN uint32_t req_id, IN bool last)
	{
		std_lock_guard lock(m_mutex);
		AsyncRequest::ptr async_;
		auto it = m_async_manager.find(req_id);
		if (it != m_async_manager.end())
		{
			async_ = it->second;
			if (last) m_async_manager.erase(it);
		}
		return async_;
	}
	inline void erase_async(IN const uint32_t req_id)
	{
		std_lock_guard lock(m_mutex);
		m_async_manager.erase(req_id);
	}

public:
	// set server address.
	inline void add_address(IN eco::net::Address& addr)
	{
		std_lock_guard lock(m_mutex);
		m_balancer.add_address(addr);
	}
	inline void update_address(IN eco::net::AddressSet& addr)
	{
		std_lock_guard lock(m_mutex);
		m_balancer.update_address(addr);
	}

	// async connect to server.
	inline void async_connect();
	inline void async_connect(IN eco::net::AddressSet& addr);
	inline String logging();

	// connect to server.
	void connect(IN uint32_t millsec);
	void connect(IN eco::net::AddressSet& addr, IN uint32_t millsec);

	// close tcp client and connection.
	void close();

	// release tcp client and connection.
	void release();

	// set a tick timer to do some work in regular intervals.
	void on_connect_ok();
	void on_live(bool active = false);
	void on_live_timeout();

	// async send data.
	inline void send(IN eco::String& data, IN uint32_t start)
	{
		std_lock_guard lock(m_mutex);
		peer().send(data, start);
	}

	inline void send(IN MessageMeta& meta)
	{
		std_lock_guard lock(m_mutex);
		peer().send(meta);
	}

	// async send data.
	inline void on_send_heartbeat()
	{
		std_lock_guard lock(m_mutex);
		if (m_option.heartbeat_rhythm())
			peer().send_heartbeat();
		else
			peer().send_live_heartbeat();
	}

	inline eco::Result request(
		IN MessageMeta& req,
		IN Codec& err_codec,
		IN Codec& rsp_codec,
		IN std::vector<void*>* rsp_set);

	inline void async(
		IN MessageMeta& req,
		IN ResponseFunc&& rsp_func);

	template<typename codec_t, typename err_t, typename rsp_t>
	inline void async(
		IN uint32_t req_type, IN const void* req,
		IN std::function<void(err_t&, eco::net::Context&)>&& reject,
		IN std::function<void(rsp_t&, eco::net::Context&)>&& resolve)
	{
		codec_t req_codec(req);
		MessageMeta req_meta;
		req_meta.codec(req_codec).message_type(req_type);
		return async(req_meta, context_func<codec_t, err_t, rsp_t>(
			reject, resolve));
	}

public:
	// when peer has connected to server.
	void on_connect(bool error);

	// load data when peer connected.
	void on_load_data();
	void on_load_locale(eco::atomic::State& state);

	// when peer has received a message data bytes.
	void on_read(IN void* peer, IN MessageTcp& head, IN eco::String& data);

	// when peer has sended a data, async notify sended data size.
	void on_send(IN void* peer, IN uint32_t size);

	// when peer has been closed.
	void on_close(IN void* peer);

	inline const char* websocket_key() const
	{
		return "bysCZJDozDYNAXgr7lCo32QsjgE=";
	}

	// whether dispatch mode, else recv mode.
	inline bool dispatch_mode() const
	{
		return m_dispatch.handler().dispatch_mode();
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#endif