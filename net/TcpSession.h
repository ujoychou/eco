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
#include <eco/net/TcpConnection.h>
#include <memory>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
// client access user handler, and user access client.
typedef std::shared_ptr<int> ClientUser;
typedef std::weak_ptr<int> ClientUserObserver;
// tcp session owner: tcp client or tcp server.
class TcpClientImpl;
class TcpServerImpl;
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
	// #.event: after server session connect.
	inline SessionData(
		IN const eco::net::SessionId id,
		IN const eco::net::TcpConnection& conn)
		: m_id(id), m_conn(conn)
	{}

	// #.event: after server session close.
	virtual ~SessionData() = 0 {}

	// get user id.
	virtual const uint64_t get_user_id() const
	{
		return 0;
	}

	// get user name.
	virtual const char* get_user_name() const
	{
		return nullptr;
	}

	// get connection object.
	inline TcpConnection& connection()
	{
		return m_conn;
	}
	inline const TcpConnection& get_connection() const
	{
		return m_conn;
	}

	inline const SessionId get_id() const
	{
		return m_id;
	}

	// async send message.
	inline void async_send(IN const MessageMeta& meta)
	{
		m_conn.async_send(meta);
	}

	// async send message.
	inline void async_send(
		IN Codec& codec,
		IN const uint32_t type,
		IN const bool last = true,
		IN const bool encrypted = true)
	{
		m_conn.async_send(codec, type, m_id, last, encrypted);
	}

#ifndef ECO_NO_PROTOBUF
	// async send protobuf.
	inline void async_send(
		IN const google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const bool last = true,
		IN const bool encrypted = true)
	{
		m_conn.async_send(msg, type, m_id, encrypted, last);
	}
#endif

private:
	SessionId		m_id;
	TcpConnection	m_conn;
	friend class SessionDataOuter;
};



////////////////////////////////////////////////////////////////////////////////
// default session factory function.
template<typename SessionDataT>
inline static SessionData* make_session_data(
	IN const SessionId session_id,
	IN const TcpConnection& connection)
{
	return new SessionDataT(session_id, connection);
}

// set session factory to create session of tcp server peer.
typedef SessionData* (*MakeSessionDataFunc)(
	IN const SessionId session_id,
	IN const TcpConnection& connection);



////////////////////////////////////////////////////////////////////////////////
class TcpSessionImpl
{
public:
	TcpConnection		m_conn;

	// session data
	SessionId			m_session_id;		// session id.
	SessionData::wptr	m_session_wptr;		// session data.
	
	// session owner: client or server.
	TcpSessionOwner		m_owner;
	ClientUser			m_user;

	inline TcpSessionImpl() : m_session_id(none_session)
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
	inline SessionData::ptr data() const;

	// get and cast session data.
	template<typename SessionDataT>
	inline std::shared_ptr<SessionDataT> cast();

public:
#ifndef ECO_NO_PROTOBUF
	// async send protobuf.
	inline void async_send(
		IN const google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const bool encrypted = true)
	{
		ProtobufCodec codec(msg);
		MessageMeta meta(codec, get_id(), type, encrypted);
		async_send(meta);
	}

	// async send protobuf authority info.
	inline void async_auth(
		IN const google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const bool encrypted = true)
	{
		ProtobufCodec codec(msg);
		MessageMeta meta(codec, none_session, type, encrypted);
		async_auth(meta);
	}

	// async send response to client by context.
	inline void async_response(
		IN const google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const Context& context,
		IN const bool last = true,
		IN const bool encrypted = true)
	{
		ProtobufCodec codec(msg);
		async_response(codec, type, context, last, encrypted);
	}
#endif

	// async response message.
	inline void async_response(
		IN Codec& codec,
		IN const uint32_t type,
		IN const Context& context,
		IN const bool last = true,
		IN const bool encrypted = true);


	// async send message.
	inline void async_send(IN const MessageMeta& meta);

	// async send authority info.
	inline void async_auth(IN const MessageMeta& meta);

private:
	// implement.
	TcpSessionImpl m_impl;
	friend class TcpSessionOuter;
};
}}

#include <eco/net/TcpSession.inl>
////////////////////////////////////////////////////////////////////////////////
#endif