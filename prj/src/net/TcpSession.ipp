#ifndef ECO_NET_TCP_SESSION_IPP
#define ECO_NET_TCP_SESSION_IPP
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
#include <eco/net/TcpPeer.h>
#include <eco/net/Ecode.h>
#include <eco/net/TcpClient.h>
#include <eco/Object.h>
#include "TcpServer.ipp"
#include "TcpPeer.ipp"


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpSession::Impl
{
public:
	
	SessionId			m_session_id;
	SessionData::wptr	m_session_wptr;	// tcp session life management.
	TcpSessionHost		m_host;			// tcp server or tcp client.

	// client mode: control the user object.
	SharedSubject		m_user;
	Protocol*			m_prot;

	inline Impl(): m_session_id(none_session)
	{}

	inline void init(IN TcpSession&)
	{}

	inline void user_born()
	{
		m_user.reset(new int(0));
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif