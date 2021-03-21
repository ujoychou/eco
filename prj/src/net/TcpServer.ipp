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
#include <eco/Eco.h>
#include <eco/thread/Map.h>
#include <eco/thread/Atomic.h>
#include <eco/thread/TimingWheel.h>
#include <eco/net/TcpServer.h>
#include <eco/net/TcpAcceptor.h>
#include "TcpStatistics.h"
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
	TcpStatistics m_statistics;
	TcpPeerHandler m_peer_handler;

	// dispatch server.
	DispatchServerPool m_dispatch_pool;

	// connection timer
	TimingWheel::Timer m_timer_close;
	TimingWheel::Timer m_timer_heart;

	// close event.
	OnClose  m_on_close;
	OnAccept m_on_accept;
	
	// session data management.
	typedef std::set<SessionId> SessionSet;
	typedef std::shared_ptr<SessionSet> SessionSetPtr;
	uint32_t m_session_ts;
	uint32_t m_session_seq;
	MakeSessionData m_make_session;

	// connection session data.
	mutable std_mutex m_session_mutex;
	std::unordered_map<SessionId, SessionSetPtr> m_conn_session;
	std::unordered_map<SessionId, SessionData::ptr> m_session_map;

public:
	inline Impl()
		: m_make_session(0)
		, m_statistics(m_option)
	{
		m_session_ts = 0;
		m_session_seq = 0;
	}

	inline ~Impl()
	{
		stop();
	}

	inline TcpServer& server()
	{
		return m_acceptor.server();
	}

	// eco implment object init by api object(parent).
	inline void init(TcpServer& server)
	{
		m_acceptor.set_server(server);

		// event hander: on accept 
		m_acceptor.register_on_accept(std::bind(&Impl::on_accept, this,
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
	// add new session.
	SessionData::ptr add_session(
		OUT SessionId& id, 
		IN const TcpConnection& conn);

	// find exist session.
	inline SessionData::ptr find_session(IN SessionId id) const
	{
		std_lock_guard lock(m_session_mutex);
		auto it = m_session_map.find(id);
		return it != m_session_map.end() ? it->second : nullptr;
	}

	// make next session id.
	inline uint64_t make_session_id()
	{
		return eco::date_time::make_id_by_ver1(
			m_option.horizental_number(), m_session_ts, m_session_seq);
	}

	// erase session.
	inline void erase_session(IN SessionId sess_id, IN SessionId conn_id)
	{
		std_lock_guard lock(m_session_mutex);
		auto it = m_session_map.find(sess_id);
		if (it != m_session_map.end())
		{
			m_session_map.erase(it);
		}
		erase_conn_session_raw(conn_id, sess_id);
	}

	// add conn session
	inline void add_conn_session(
		IN SessionId conn_id,
		IN SessionId sess_id,
		IN SessionData::ptr new_sess)
	{
		std_lock_guard lock(m_session_mutex);
		SessionSetPtr& map = m_conn_session[conn_id];
		if (map == nullptr) map.reset(new SessionSet);
		(*map).insert(sess_id);
		m_session_map[sess_id] = new_sess;
	}

	// erase conn session
	inline bool erase_conn_session(SessionId conn_id, SessionId sess_id)
	{
		std_lock_guard lock(m_session_mutex);
		return erase_conn_session_raw(conn_id, sess_id);
	}
	inline bool erase_conn_session_raw(SessionId conn_id, SessionId sess_id)
	{
		auto it = m_conn_session.find(conn_id);
		if (it != m_conn_session.end())
		{
			it->second->erase(sess_id);
			return true;
		}
		return false;
	}

	// erase conn session
	inline void clear_conn_session(IN SessionId id)
	{
		std_lock_guard lock(m_session_mutex);
		auto itc = m_conn_session.find(id);
		if (itc != m_conn_session.end())
		{
			auto& sess_map = itc->second;
			for (auto it = sess_map->begin(); it != sess_map->end(); ++it)
			{
				m_session_map.erase(*it);
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
			// destroy all io resource(should call constructor) before io server.
			// close peer set before server stop.
			m_session_map.clear();
			m_statistics.clear();
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

	// on call.
	void on_accept(IN TcpPeer::ptr& p, bool error);

/////////////////////////////////////////////////////////////// TCP PEER HANDLER
public:
	// when peer has received a message data bytes.
	void on_read(IN void* peer, IN MessageHead& head, IN eco::String& data);

	// when peer has sended a data, async notify sended data size.
	void on_send(IN void* peer, IN uint32_t size);

	// when language and user authorize.
	void on_auth(const char* lang, const char* user, TcpPeer::ptr& peer);

	// when peer has been closed.
	void on_close(IN void* peer);
};



////////////////////////////////////////////////////////////////////////////////
}}
#endif