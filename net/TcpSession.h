#ifndef ECO_NET_TCP_CLIENT_SESSION_H
#define ECO_NET_TCP_CLIENT_SESSION_H
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
#include <eco/net/SessionData.h>
#include <eco/net/protocol/Protocol.h>
#include <memory>


namespace eco{;
namespace net{;
class TcpPeer;
////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpSession
{
	ECO_SHARED_API(TcpSession)
public:
	// open session and create session data object.
	void open();

	// check whether session has been opened.
	bool opened() const;

	// close session, release session data.
	void close();

	// set session info.
	void set_session(
		IN TcpPeer& peer,
		IN const SessionId session_id);

	// get protocol in peer.
	Protocol& protocol();
	ProtocolHead& protocol_head();

	// get session data.
	uint32_t get_session_id() const;

	// get and cast session data.
	template<typename SessionDataT>
	SessionDataT* cast()
	{
		return static_cast<SessionDataT*>(data());
	}
	// get session data.
	SessionData* data();
	
	// send message.
	void send(IN eco::String& data);

	/*@ send message.
	* @ para.meta: the discription of message.
	*/
	void send(IN MessageMeta& meta)
	{
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif