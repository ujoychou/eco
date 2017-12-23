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
class TcpClient;
class TcpServer;
class Protocol;
class ProtocolHead;
////////////////////////////////////////////////////////////////////////////////
typedef uint32_t SessionId;
const SessionId none_session = 0;

// 1.tcp session and tcp client life management.
// 2.user and service life manangement.
typedef std::shared_ptr<int> SharedSubject;
typedef std::weak_ptr<int> SharedObserver;


////////////////////////////////////////////////////////////////////////////////
enum
{
	tcp_session_host_server = 1,
	tcp_session_host_client = 2,
};
typedef uint32_t TcpSessionHostType;
typedef void* TcpSessionHostPtr;

// tcp peer host: tcp client or tcp server.
class TcpSessionHost
{
public:
	inline TcpSessionHost()
		: m_type(0), m_host(nullptr), m_peer(nullptr)
	{}

	inline TcpSessionHost(IN TcpClient& client)
		: m_type(tcp_session_host_client)
		, m_host(&client), m_peer(nullptr)
	{}

	inline TcpSessionHost(
		IN TcpServer& server, 
		IN TcpPeer* peer = nullptr)
		: m_type(tcp_session_host_server)
		, m_host(&server), m_peer(peer)
	{}

	inline void set_peer(IN TcpPeer& peer)
	{
		m_peer = &peer;
	}

	inline void clear()
	{
		m_type = 0;
		m_host = 0;
		m_peer = 0;
	}

	bool response_heartbeat() const;
	ProtocolHead* protocol_head() const;

	TcpSessionHostType	m_type;
	TcpSessionHostPtr	m_host;
	TcpPeer*			m_peer;
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