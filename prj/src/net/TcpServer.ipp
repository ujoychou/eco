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
#include <eco/Project.h>
#include <eco/net/TcpServer.h>
#include <eco/net/IoTimer.h>
#include <eco/net/TcpAcceptor.h>
#include <eco/net/DispatchServer.h>
#include <eco/net/protocol/ProtocolHead.h>
#include <eco/thread/Map.h>
#include <eco/thread/Atomic.h>
#include <map>
#include <vector>
#include "TcpPeerSet.h"


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpServer::Impl : public TcpPeerHandler
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
	// protocol.
	typedef std::shared_ptr<Protocol> ProtocolPtr;
	std::auto_ptr<ProtocolHead> m_prot_head;
	std::map<uint32_t, ProtocolPtr> m_protocol_set;

	// tcp acceptor.
	TcpAcceptor m_acceptor;
	TcpPeerSet m_peer_set;
	IoTimer m_timer;
	MakeConnectionDataFunc m_make_connection;

	// dispatch server.
	DispatchServer m_dispatch;

	// session data management.
	MakeSessionDataFunc m_make_session;
	eco::Atomic<SessionId> m_next_session_id;
	std::vector<SessionId> m_left_session_ids;
	eco::HashMap<SessionId, SessionData::ptr> m_session_map;

public:
	inline Impl() : m_make_connection(nullptr), m_make_session(nullptr)
	{
		m_next_session_id = none_session;
	}

	inline ~Impl()
	{}

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

	// protocol head.
	inline void set_protocol_head(IN ProtocolHead* v)
	{
		return m_prot_head.reset(v);
	}
	
	// read only and thread safe.
	inline void set_protocol(IN Protocol* p)
	{
		m_protocol_set.clear();
		m_protocol_set[p->version()] = ProtocolPtr(p);
	}
	inline void register_protocol(IN Protocol* p)
	{
		m_protocol_set[p->version()] = ProtocolPtr(p);
	}

	// get protocol.
	inline Protocol* protocol(IN const uint32_t version) const
	{
		auto it = m_protocol_set.find(version);
		if (it != m_protocol_set.end())
		{
			return it->second.get();
		}
		return nullptr;
	}

	// is it a session mode.
	inline bool session_mode() const
	{
		return (m_make_session != nullptr);
	}
	
//////////////////////////////////////////////////////////////////////// SESSION
public:
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


	// add new session.
	SessionData::ptr add_session(OUT SessionId& id, IN TcpPeer& peer);

	// find exist session.
	inline SessionData::ptr find_session(IN const SessionId id) const
	{
		SessionData::ptr v;
		m_session_map.find(v, id);
		return v;
	}

	// erase session.
	inline void erase_session(IN const SessionId id)
	{
		SessionData::ptr v = m_session_map.pop(id);
		if (v != nullptr)
			eco::thread::release(v);
		eco::Mutex::ScopeLock lock(m_session_map.mutex());
		m_left_session_ids.push_back(id);
	}

////////////////////////////////////////////////////////////////////// IO SERVER
public:
	// run server.
	void start();

	// stop server.
	inline void stop()
	{
		m_timer.cancel();
		// destroy all io resource(should call constructor) before io server.
		// close peer set before server stop.
		m_peer_set.clear();
		// close acceptor and stop io server.
		m_acceptor.stop();
		// stop business server.
		m_dispatch.stop();
	}

	// wait server thread end.
	inline void join()
	{
		m_dispatch.join();
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
			m_peer_set.send_rhythm_heartbeat(*m_prot_head);
		else
			m_peer_set.send_live_heartbeat(*m_prot_head);
	}

	// on call.
	void on_timer(IN const eco::Error* e);
	void on_accept(IN TcpPeer::ptr& p, IN const eco::Error* e);

/////////////////////////////////////////////////////////////// TCP PEER HANDLER
public:
	// when peer has received a message data bytes.
	virtual void on_read(
		IN void* peer,
		IN eco::String& data) override;

	// when peer has sended a data, async notify sended data size.
	virtual void on_send(
		IN void* peer,
		IN const uint32_t size) override;

	// when peer has been closed.
	virtual void on_close(
		IN uint64_t peer_id) override;

	// get protocol head.
	virtual ProtocolHead& protocol_head() const override
	{
		return *m_prot_head;
	}

	// whether is a websocket.
	virtual bool websocket() const override
	{
		return m_option.websocket();
	}
};



////////////////////////////////////////////////////////////////////////////////
}}
#endif