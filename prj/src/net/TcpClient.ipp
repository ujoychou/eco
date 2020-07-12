#ifndef ECO_NET_TCP_CLIENT_IPP
#define ECO_NET_TCP_CLIENT_IPP
#include "PrecHeader.h"
#include <eco/net/TcpClient.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/thread/State.h>
#include <eco/thread/Map.h>
#include <eco/thread/Monitor.h>
#include <eco/net/Worker.h>
#include <eco/net/IoTimer.h>
#include <eco/net/protocol/WebSocketProtocol.h>
#include "TcpPeer.ipp"
#include "DispatchServer.h"


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

	inline void release()
	{
		m_peer->close();
		// release peer before io service stop.
		m_peer.reset();
	}

	void add_address(IN Address& addr);
	void update_address(IN AddressSet& addr);
	bool connect();		// connect to server by blance algorithm.
};


////////////////////////////////////////////////////////////////////////////////
class SessionDataPack
{
	ECO_OBJECT(SessionDataPack);
public:
	eco::String m_request;
	uint32_t m_request_start;
	SessionData::ptr m_session;
	ClientUserObserver m_user_observer;
	uint64_t m_request_data;
	uint32_t m_auto_login;

public:
	inline SessionDataPack(IN bool auto_login = false)
		: m_request_data(0), m_auto_login(auto_login), m_request_start(0)
	{}
};


////////////////////////////////////////////////////////////////////////////////
class SyncRequest : public eco::Object<SyncRequest>
{
public:
	inline SyncRequest(
		IN Codec* err_codec,
		IN Codec* rsp_codec,
		IN std::vector<void*>* rsp_set)
		: m_err_codec(err_codec)
		, m_rsp_codec(rsp_codec)
		, m_rsp_set(rsp_set)
		, m_error_id(0)
	{}

	// if decode "error or response" message fail. return false, else true.
	inline bool decode(const eco::Bytes& msg)
	{
		// 1.parse reject error object.
		if (m_error_id != 0)
		{
			if (m_err_codec && !m_err_codec->decode(msg.m_data, msg.m_size))
				return false;					// #.decode error msg fail.
		}

		// 2.parse response object or response_set.
		if (msg.m_size > 0 && m_rsp_codec)
		{
			void* obj = m_rsp_codec->decode(msg.m_data, msg.m_size);
			if (obj == nullptr) return false;	// #.decode rsp msg fail.
			if (m_rsp_set) m_rsp_set->push_back(obj);
		}
		return true;
	}

	uint32_t	m_error_id;
	Codec*		m_err_codec;
	Codec*		m_rsp_codec;
	std::vector<void*>* m_rsp_set;
	eco::Monitor m_monitor;
};
////////////////////////////////////////////////////////////////////////////////
class AsyncRequest : public eco::Object<AsyncRequest>
{
public:
	inline AsyncRequest(std::function<void(eco::net::Context& c)>& f)
		: m_func(f) {}

	std::function<void(eco::net::Context&)> m_func;
};


////////////////////////////////////////////////////////////////////////////////
class TcpClient::Impl
{
	ECO_IMPL_INIT(TcpClient);
public:
	LoadBalancer	m_balancer;		// load balance for multi server.
	TcpClientOption	m_option;		// client option.
	ProtocolFamily  m_protocol;		// client protocol.
	TcpPeerHandler	m_peer_handler;
	
	// io server, timer and business dispatcher server.
	eco::net::Worker	m_worker;	// io thread.
	eco::net::IoTimer	m_timer;	// run in io thread.
	DispatchServer		m_dispatch;	// dispatcher thread.

	// connection event.
	ClientOpenFunc  m_on_open;
	ClientCloseFunc m_on_close;

	// session data management.
	MakeSessionDataFunc m_make_session;
	MakeConnectionDataFunc m_make_connection;
	// all session that client have, diff by "&session".
	std::unordered_map<void*, SessionDataPack::ptr> m_authority_map;
	// connected session that has id build by server.
	eco::HashMap<uint32_t, SessionDataPack::ptr> m_session_map;
	mutable eco::Mutex	m_mutex;

	// async management.
	eco::Atomic<uint32_t> m_request_id;
	uint32_t m_timeout_millsec;
	std::unordered_map<uint32_t, SyncRequest::ptr> m_sync_manager;
	std::unordered_map<uint32_t, AsyncRequest::ptr> m_async_manager;

public:
	inline Impl() 
		: m_make_session(nullptr)
		, m_make_connection(nullptr)
		, m_on_open(nullptr)
		, m_on_close(nullptr)
		, m_timeout_millsec(5000)
	{}

	inline ~Impl()
	{
		close();
	}

	// get current tcp peer connected to server.
	inline TcpPeer::ptr& peer()
	{
		return m_balancer.m_peer;
	}

	// open a session with no authority.
	inline TcpSession open_session();

	// set a authority.
	inline void set_authority(IN void* key, IN SessionDataPack::ptr& pack)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		m_authority_map[key] = pack;
	}

	// find exist authority.
	inline SessionDataPack::ptr find_authority(IN void* key)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		auto it = m_authority_map.find(key);
		return it != m_authority_map.end() ? it->second : nullptr;
	}

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
		IN const uint32_t req_id,
		IN Codec* err_codec,
		IN Codec* rsp_codec,
		IN std::vector<void*>* rsp_set = nullptr)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		auto& ptr = m_sync_manager[req_id];
		if (ptr == nullptr)
			ptr.reset(new SyncRequest(err_codec, rsp_codec, rsp_set));
		return ptr;
	}

	// async management pop item.
	inline SyncRequest::ptr pop_sync(IN uint32_t req_id, IN bool last)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
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
		eco::Mutex::ScopeLock lock(m_mutex);
		m_sync_manager.erase(req_id);
	}

public:
	// async management post item.
	inline AsyncRequest::ptr post_async(
		IN const uint32_t req_id,
		IN std::function<void(eco::net::Context&)>& rsp_func)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		auto& ptr = m_async_manager[req_id];
		if (ptr == nullptr)
			ptr.reset(new AsyncRequest(rsp_func));
		return ptr;
	}

	// async management pop item.
	inline AsyncRequest::ptr pop_async(IN uint32_t req_id, IN bool last)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		AsyncRequest::ptr async;
		auto it = m_async_manager.find(req_id);
		if (it != m_async_manager.end())
		{
			async = it->second;
			if (last) m_async_manager.erase(it);
		}
		return async;
	}
	inline void erase_async(IN const uint32_t req_id)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		m_async_manager.erase(req_id);
	}

public:
	// set server address.
	inline void add_address(IN eco::net::Address& addr)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		m_balancer.add_address(addr);
	}
	inline void update_address(IN eco::net::AddressSet& addr)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		m_balancer.update_address(addr);
	}

	// async connect to server.
	inline void async_connect(IN bool reconnect);
	inline void async_connect(IN eco::net::AddressSet& addr, IN bool reconnect)
	{
		update_address(addr);
		// reconnect to new address if cur address is removed.
		async_connect(reconnect);
	}
	inline String logging();

	// release tcp client and connection.
	void close();

	// set a tick timer to do some work in regular intervals.
	inline void set_tick_timer()
	{
		if (m_option.has_tick_timer())
			m_timer.set_timer(m_option.get_tick_time());
	}
	void on_timer(IN const eco::Error* e);

	// check peer live.
	inline void check_peer_live()
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		if (peer() && !peer()->impl().check_peer_live())
		{
			eco::Error e(e_peer_lost, "peer lost server heartbeat.");
			peer()->impl().close_and_notify(e, true);
		}
	}

	// async send data.
	inline void send(IN eco::String& data, IN const uint32_t start)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		if (peer())
			peer()->impl().send(data, start);
	}

	inline void send(IN MessageMeta& meta)
	{
		eco::Error e;
		eco::String data;
		uint32_t start = 0;
		if (m_protocol.protocol_latest()->encode(data, start, meta, e))
		{
			send(data, start);
		}
	}

	// async send data.
	inline void send_heartbeat()
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		if (peer())
			peer()->impl().send_heartbeat(*m_protocol.protocol_latest());
	}

	inline void send(IN SessionDataPack::ptr& pack)
	{
		eco::String data;
		data.assign(pack->m_request.c_str(), pack->m_request.size());

		// client isn't ready and connected when first send authority.
		// because of before that client is async connect.
		eco::Mutex::ScopeLock lock(m_mutex);
		if (peer() && peer()->impl().get_state().connected())
		{
			peer()->impl().send(data, pack->m_request_start);
		}
	}

	inline void async_auth(
		IN TcpSessionImpl& sess,
		IN MessageMeta& meta);

	inline eco::Result request(
		IN MessageMeta& req,
		IN Codec* err_codec,
		IN Codec* rsp_codec,
		IN std::vector<void*>* rsp_set);

	inline void async(
		IN MessageMeta& req,
		IN ResponseFunc& rsp_func);

public:
	// when peer has connected to server.
	void on_connect(IN const eco::Error* e);

	// when peer has received a message data bytes.
	void on_read(IN void* peer, IN MessageHead& head, IN eco::String& data);

	// when peer has sended a data, async notify sended data size.
	void on_send(IN void* peer, IN uint32_t size);

	// when peer has been closed.
	void on_close(ConnectionId peer_id, const Error& e, bool erase_peer);

	inline const char* websocket_key() const
	{
		return "bysCZJDozDYNAXgr7lCo32QsjgE=";
	}

	// whether dispatch mode, else recv mode.
	inline bool dispatch_mode() const
	{
		return m_dispatch.get_message_handler().dispatch_mode();
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#endif