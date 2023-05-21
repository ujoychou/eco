#ifndef ECO_NET_TCP_OUTER_IPP
#define ECO_NET_TCP_OUTER_IPP
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-17.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/net/TcpSession.h>
#include "TcpClient.ipp"
#include "TcpServer.ipp"



ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class SessionDataOuter
{
public:
	inline SessionDataOuter(IN SessionData& data) : m_data(data)
	{}
	inline void set_id(IN const SessionId session_id)
	{
		m_data.m_id = session_id;
	}
public:
	SessionData& m_data;
};


////////////////////////////////////////////////////////////////////////////////
class TcpSessionOuter
{
public:
	TcpSessionOuter(IN TcpSession& sess) : m_sess(sess)
	{}

	inline TcpSessionImpl& impl()
	{
		return m_sess.m_impl;
	}

	inline void make_client_session()
	{
		impl().m_user.reset(new int(0));
	}

	inline bool open(IN const SessionId session_id);

public:
	TcpSession& m_sess;
};


////////////////////////////////////////////////////////////////////////////////
class TcpConnectionOuter
{
public:
	inline TcpConnectionOuter(IN TcpConnection& conn)
		: m_conn(conn) {}

	inline void set_peer(IN TcpPeer::wptr& peer)
	{
		m_conn.m_peer = peer;
	}

	inline void set_id(IN SessionId id)
	{
		m_conn.m_id = id;
	}

	TcpConnection& m_conn;
};


////////////////////////////////////////////////////////////////////////////////
bool TcpSessionOuter::open(IN const SessionId session_id)
{
	// get the exist session.
	if (impl().m_owner.m_server)
	{
		auto* server = (TcpServer::Impl*)impl().m_owner.m_owner;
		auto sess = server->find_session(session_id);
		if (sess != nullptr)
		{
			impl().m_session_wptr = sess;
			impl().m_session_id = session_id;
			return true;
		}
	}
	else
	{
		auto* client = (TcpClient::Impl*)impl().m_owner.m_owner;
		auto pack = client->find_session(session_id);
		if (pack != nullptr)
		{
			impl().m_user = pack->m_user_observer.lock();
			if (impl().m_user != nullptr)
			{
				impl().m_session_wptr = pack->m_session;
				impl().m_session_id = session_id;
				return true;
			}
		}
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif