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
	inline void send(IN const MessageMeta& meta)
	{
		m_conn.send(meta);
	}

	// async send message.
	inline void send(IN Codec* codec, IN uint32_t type, IN bool last)
	{
		m_conn.send(codec, MessageOption(type, 0, last, m_id));
	}

	// async publish message.
	inline void publish(IN Codec* codec, IN uint32_t type)
	{
		m_conn.send(codec, MessageOption(type, 0, false, m_id));
	}

#ifndef ECO_NO_PROTOBUF
	// async send protobuf.
	inline void send(
		IN const google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const bool last)
	{
		m_conn.send(msg, MessageOption(type, 0, last, m_id));
	}

	// async publish protobuf.
	inline void publish(
		IN const google::protobuf::Message& msg,
		IN const uint32_t type)
	{
		m_conn.publish(msg, MessageOption(type, 0, false, m_id));
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
	TcpOwner			m_owner;
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
	inline bool authorize();

	// close session, release session data.
	inline void close();

	// check whether session has been opened.
	inline bool authorized() const;

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
	// async response message.
	inline void response(IN Codec*, IN MessageOption&, IN const Context&);

	// async send message.
	inline void send(IN const MessageMeta& meta);

	// async send authority info.
	inline void authorize(IN const MessageMeta& meta);

#ifndef ECO_NO_PROTOBUF
	// async send protobuf.
	inline void send(
		IN const google::protobuf::Message& msg,
		IN uint32_t type, IN bool is_last)
	{
		ProtobufCodec codec(msg);
		MessageOption opt(type, 0, is_last, get_id());
		send(MessageMeta(&codec, opt));
	}

	// async send protobuf authority info.
	inline void authorize(
		IN const google::protobuf::Message& msg,
		IN uint32_t type, IN bool is_last)
	{
		ProtobufCodec codec(msg);
		MessageOption opt(type, 0, is_last, get_id());
		authorize(MessageMeta(&codec, opt));
	}

	// async send response to client by context.
	inline void response(
		IN const google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const uint32_t error_id,
		IN const bool is_last,
		IN const Context& context)
	{
		ProtobufCodec codec(msg);
		response(&codec, MessageOption(type, error_id, is_last), context);
	}
#endif

private:
	// implement.
	TcpSessionImpl m_impl;
	friend class TcpSessionOuter;
};
}}

#include <eco/net/TcpSession.inl>
////////////////////////////////////////////////////////////////////////////////
#endif