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
#include <eco/Repository.h>
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
	TcpServer* m_server;
	TcpServerOption m_option;

	// tcp acceptor.
	uint32_t m_port;
	TcpAcceptor m_acceptor;
	TcpPeerSet m_peer_set;
	IoTimer m_timer;

	// protocol.
	typedef std::shared_ptr<Protocol> ProtocolPtr;
	std::auto_ptr<ProtocolHead> m_prot_head;
	std::map<uint32_t, ProtocolPtr> m_protocol_set;

	// dispatch server.
	DispatchServer m_dispatch;

	// session management.
	MakeSessionDataFunc m_make_session;
	eco::Atomic<SessionId> m_next_session_id;
	std::vector<SessionId> m_left_session_ids;
	eco::Repository<SessionId, SessionData::ptr> m_session_map;

public:
	inline Impl()
	{
		m_port = min_port;
		m_server = nullptr;
		m_make_session = nullptr;
		m_next_session_id = none_session;
	}

	// eco implment object init by api object(parent).
	inline void init(TcpServer& server)
	{
		m_server = &server;
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
	
public:
	// session maker which is a session factory method.
	inline void set_session_data(IN MakeSessionDataFunc v)
	{
		m_make_session = v;
	}

	// produce a new session id.
	inline uint32_t next_session_id();

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
		{
			eco::thread::release(v);
		}
	}

public:
	// run server.
	void start();

	// stop server.
	void stop();

	// wait server thread end.
	void join();

	/*@ start accept.
	* @ para.port: listen port which to accept new client tcp_connection.
	*/
	void listen(
		IN  const uint16_t port,
		IN  const uint32_t io_server_size);

	/*@ ready to accept client tcp_connection.*/
	void async_accept();

	/*@ start tick timer that do some auxiliary.*/
	void set_tick_timer();

	/*@ send heartbeat to peer of all connection.*/
	void send_heartbeat();

	/*@ clean timeout request that has asynchronous handled by app server.*/
	void clean_timeout_request();

	// on call.
	void on_timer(IN const eco::Error* e);
	void on_accept(IN TcpPeer::ptr& p, IN const eco::Error* e);

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

	

	
};



////////////////////////////////////////////////////////////////////////////////
}}
#endif