#ifndef ECO_NET_TCP_SESSION_H
#define ECO_NET_TCP_SESSION_H
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
#include <eco/net/TcpConnection.h>
#include <memory>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class TcpSessionImpl
{
public:
	TcpConnection		m_conn;

	// session data
	SessionId			m_session_id;		// session id.
	SessionData::wptr	m_session_wptr;		// session data.
	TcpPeer*			m_peer;				// sesseion peer.
	
	// session owner: client or server.
	TcpSessionOwner		m_owner;
	ClientUser			m_user;

	inline TcpSessionImpl() : m_session_id(none_session), m_peer(nullptr)
	{}
};


////////////////////////////////////////////////////////////////////////////////
class Context;
class TcpSession
{
public:
	// open session.
	inline bool auth();

	// close session, release session data.
	inline void close();

	// check whether session has been opened.
	inline bool authed() const;

	// get connection.
	inline TcpConnection& connection();
	inline const TcpConnection& get_connection() const;

	// get session data.
	inline const SessionId get_id() const;

	// get session data.
	inline SessionData::ptr data();

	// get and cast session data.
	template<typename SessionDataT>
	inline std::shared_ptr<SessionDataT> cast();

	// check that it is a session mode.
	inline bool session_mode() const;

public:
#ifndef ECO_NO_PROTOBUF
	// async send protobuf.
	inline void async_send(
		IN google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const MessageCategory category = category_message)
	{
		ProtobufCodec codec(msg);
		MessageMeta meta(codec, get_id(), type, category);
		async_send(meta);
	}

	// async send protobuf authority info.
	inline void async_auth(
		IN google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const MessageCategory category = category_message)
	{
		ProtobufCodec codec(msg);
		MessageMeta meta(codec, none_session, type, category);
		async_auth(meta);
	}

	// async send response to client by context.
	inline void async_resp(
		IN google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const Context& context,
		IN const bool last = false)
	{
		ProtobufCodec codec(msg);
		async_resp(codec, type, context, last);
	}
#endif

	// async response message.
	inline void async_resp(
		IN Codec& codec,
		IN const uint32_t type,
		IN const Context& context,
		IN const bool last = false);

private:
	// async send message.
	inline void async_send(IN MessageMeta& meta);

	// async send authority info.
	inline void async_auth(IN MessageMeta& meta);

	// implement.
	TcpSessionImpl m_impl;
	friend class TcpSessionOuter;
};
}}

#include <eco/net/TcpSession.inl>
////////////////////////////////////////////////////////////////////////////////
#endif