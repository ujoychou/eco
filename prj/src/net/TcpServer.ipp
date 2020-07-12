#ifndef ECO_NET_TCP_SERVER_IPP
#define ECO_NET_TCP_SERVER_IPP
/*******************************************************************************
@ name


@ function


@ exception


@ remark


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-12.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <set>
#include <eco/thread/Map.h>
#include <eco/thread/Atomic.h>
#include <eco/net/IoTimer.h>
#include <eco/net/TcpServer.h>
#include <eco/net/TcpAcceptor.h>
#include "TcpPeerSet.h"
#include "DispatchServer.h"


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpServer::Impl
{
public:
	enum
	{
		min_port = 30000,
		max_port = 40000,
		max_conn_size = 10000,
		max_sess_size = max_conn_size * 1000,
	};

	// server option.
	TcpServerOption m_option;
	ProtocolFamily m_protocol;

	// tcp acceptor.
	TcpAcceptor m_acceptor;
	TcpPeerSet m_peer_set;
	IoTimer m_timer;
	MakeConnectionDataFunc m_make_connection;
	TcpPeerHandler m_peer_handler;
	// dispatch server.
	DispatchServerPool m_dispatch_pool;

	// close event.
	ServerAcceptFunc m_on_accept;
	ServerCloseFunc  m_on_close;
	
	// session data management.
	MakeSessionDataFunc m_make_session;
	eco::Atomic<SessionId> m_next_session_id;
	std::vector<SessionId> m_left_session_ids;
	eco::HashMap<SessionId, SessionData::ptr> m_session_map;
	// connection session data.
	typedef std::set<SessionId> SessionSet;
	typedef std::shared_ptr<SessionSet> SessionSetPtr;
	eco::Mutex m_conn_session_mutex;
	std::unordered_map<ConnectionId, SessionSetPtr> m_conn_session;

public:
	inline Impl() : m_make_connection(nullptr), m_make_session(nullptr)
	{
		m_next_session_id = none_session;
	}

	inline ~Impl()
	{
		stop();
	}

	// eco implment object init by api object(parent).
	inline void init(TcpServer& server)
	{
		m_acceptor.set_server(server);

		// event hander: on timer
		m_timer.register_on_timer(
			std::bind(&Impl::on_timer, this, std::placeholders::_1));

		// event hander: on accept 
		m_acceptor.register_on_accept(
			std::bind(&Impl::on_accept, this,
				std::placeholders::_1, std::placeholders::_2));
	}

	inline bool receive_mode() const
	{
		return m_dispatch_pool.get_message_handler().receive_mode();
	}

	inline const char* websocket_key() const
	{
		return "";
	}

	// whether dispatch mode, else recv mode.
	inline bool dispatch_mode() const
	{
		return m_dispatch_pool.get_message_handler().dispatch_mode();
	}
	
//////////////////////////////////////////////////////////////////////// SESSION
public:
	// produce a new session id.
	inline uint32_t get_peer_size() const
	{
		return (uint32_t)m_peer_set.get_connection_size();
	}

	// produce a new session id.
	inline uint32_t next_session_id()
	{
		eco::Mutex::ScopeLock lock(m_session_map.mutex());
		// reuse session id that has been recycled.
		if (m_left_session_ids.size() > 0)
		{
			uint32_t id = m_left_session_ids.back();
			m_left_session_ids.pop_back();
			return id;
		}
		// session will not reach the "uint32_t" max value.
		return ++m_next_session_id;
	}

	// add new connection.
	inline void set_valid_peer(IN TcpPeer& peer, IN Protocol* prot)
	{
		// create connection data.
		if (m_make_connection && !peer.impl().m_data.get())
		{
			peer.impl().make_connection_data(m_make_connection, prot);
		}

		// set valid state which peer is a dos peer.
		if (!peer.impl().get_state().valid())
		{
			m_peer_set.set_valid_peer(peer.impl().get_id());
			peer.impl().state().set_valid(true);
		}
	}

	// add new session.
	SessionData::ptr add_session(
		OUT SessionId& id, 
		IN const TcpConnection& conn);

	// find exist session.
	inline SessionData::ptr find_session(IN const SessionId id) const
	{
		SessionData::ptr v;
		m_session_map.find(v, id);
		return v;
	}

	// erase session.
	inline void erase_session(
		IN const SessionId sess_id, 
		IN const ConnectionId conn_id)
	{
		{
			eco::Mutex::ScopeLock lock(m_session_map.mutex());
			auto it = m_session_map.map().find(sess_id);
			if (it != m_session_map.map().end())
			{
				auto v = it->second;
				m_session_map.map().erase(it);
				m_left_session_ids.push_back(sess_id);
			}
		}
		erase_conn_session(conn_id, sess_id);
	}

	// add conn session
	inline void add_conn_session(
		IN const ConnectionId conn_id, 
		IN const SessionId sess_id)
	{
		eco::Mutex::ScopeLock lock(m_conn_session_mutex);
		SessionSetPtr& map = m_conn_session[conn_id];
		if (map == nullptr)
		{
			map.reset(new SessionSet);
		}
		(*map).insert(sess_id);
	}

	// erase conn session
	inline bool erase_conn_session(
		IN const ConnectionId conn_id,
		IN const SessionId sess_id)
	{
		eco::Mutex::ScopeLock lock(m_conn_session_mutex);
		auto it = m_conn_session.find(conn_id);
		if (it != m_conn_session.end())
		{
			it->second->erase(sess_id);
			return true;
		}
		return false;
	}

	// erase conn session
	inline void clear_conn_session(IN ConnectionId conn_id)
	{
		eco::Mutex::ScopeLock lock(m_conn_session_mutex);
		auto itc = m_conn_session.find(conn_id);
		if (itc != m_conn_session.end())
		{
			auto& sess_map = itc->second;
			for (auto it = sess_map->begin(); it != sess_map->end(); ++it)
			{
				m_session_map.erase(*it);
				m_left_session_ids.push_back(*it);
			}
			m_conn_session.erase(itc);
		}
	}


////////////////////////////////////////////////////////////////////// IO SERVER
public:
	// run server.
	inline void start();

	// stop server.
	inline void stop()
	{
		if (m_acceptor.running())
		{
			m_timer.close();
			// destroy all io resource(should call constructor) before io server.
			// close peer set before server stop.
			m_session_map.clear();
			m_peer_set.clear();
			// close acceptor and stop io server.
			m_acceptor.stop();
			// stop business server.
			m_dispatch_pool.close();
		}
	}

	// wait server thread end.
	inline void join()
	{
		m_dispatch_pool.join();
		m_acceptor.join();
	}

	// set a tick timer to do some work in regular intervals.
	inline void set_tick_timer()
	{
		if (m_option.has_tick_timer())
			m_timer.set_timer(m_option.get_tick_time());
	}

	/*@ send heartbeat to peer of all connection.*/
	inline void async_send_heartbeat()
	{
		if (m_option.rhythm_heartbeat())
			m_peer_set.send_rhythm_heartbeat(*m_protocol.protocol_latest());
		else
			m_peer_set.send_live_heartbeat(*m_protocol.protocol_latest());
	}

	// on call.
	void on_timer(IN const eco::Error* e);
	void on_accept(IN TcpPeer::ptr& p, IN const eco::Error* e);

/////////////////////////////////////////////////////////////// TCP PEER HANDLER
public:
	// when peer has received a message data bytes.
	void on_read(IN void* peer, IN MessageHead& head, IN eco::String& data);

	// when peer has sended a data, async notify sended data size.
	void on_send(IN void* peer, IN uint32_t size);

	// when peer has been closed.
	void on_close(IN ConnectionId id, IN bool erase_peer);
};



////////////////////////////////////////////////////////////////////////////////
}}
#endif