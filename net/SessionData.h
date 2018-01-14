#ifndef ECO_NET_SESSION_DATA_H
#define ECO_NET_SESSION_DATA_H
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
#include <eco/ExportApi.h>
#include <eco/Type.h>
#include <eco/HeapOperators.h>


namespace eco{;
namespace net{;
class TcpPeer;
class TcpClientImpl;
class TcpServerImpl;
class Protocol;
class ProtocolHead;
////////////////////////////////////////////////////////////////////////////////
typedef uint32_t SessionId;
const SessionId none_session = 0;

// client access user handler, and user access client.
typedef std::shared_ptr<int> ClientUser;
typedef std::weak_ptr<int> ClientUserObserver;

// tcp session owner: tcp client or tcp server.
class TcpSessionOwner
{
public:
	inline TcpSessionOwner() : m_server(false), m_owner(nullptr)
	{}

	inline TcpSessionOwner(IN TcpClientImpl& client)
		: m_server(false), m_owner(&client)
	{}

	inline TcpSessionOwner(IN TcpServerImpl& server)
		: m_server(true), m_owner(&server)
	{}

	inline void set(IN TcpClientImpl& client)
	{
		m_server = false;
		m_owner = &client;
	}

	inline void set(IN TcpServerImpl& server)
	{
		m_server = true;
		m_owner = &server;
	}

	inline void clear()
	{
		m_server = 0;
		m_owner = 0;
	}

	uint32_t	m_server;
	void*		m_owner;
};


////////////////////////////////////////////////////////////////////////////////
// define session data holder. (using the boost::any<type> mechanism)
class SessionData : public eco::HeapOperators
{
	ECO_OBJECT(SessionData);
public:
	inline  SessionData()  {}
	virtual ~SessionData() {}
};

// default session factory function.
template<typename SessionDataT>
inline static SessionData* make_session_data(IN const SessionId session_id)
{
	return new SessionDataT();
}

// set session factory to create session of tcp server peer.
typedef SessionData* (*MakeSessionDataFunc)(IN const SessionId session_id);


////////////////////////////////////////////////////////////////////////////////
}}
#endif