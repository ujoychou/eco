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
		IN const eco::net::SessionId id_,
		IN const eco::net::TcpConnection& conn)
		: m_id(id_), m_conn(conn)
	{}

	// #.event: after server session close.
	virtual ~SessionData() = 0 {}

	// get user id.
	virtual const uint64_t user_id() const
	{
		return 0;
	}

	// get user name.
	virtual const char* user_name() const
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

	inline const SessionId id() const
	{
		return m_id;
	}

	// async send message.
	inline void send(IN MessageMeta& meta)
	{
		m_conn.send(meta.session_id(m_id));
	}

	// async send message.
	inline void send(IN Codec& cdc, IN int type, IN bool last_)
	{
		MessageMeta meta;
		meta.codec(cdc).message_type(type).last(last_);
		m_conn.send(meta);
	}

	// async publish message.
	inline void publish(IN Codec& cdc, IN int type, IN char snap_)
	{
		MessageMeta meta;
		meta.codec(cdc).message_type(type).snap(snap_);
		m_conn.send(meta);
	}

#ifndef ECO_NO_PROTOBUF
	// async send protobuf.
	inline void send(
		IN const google::protobuf::Message& msg,
		IN int type, IN bool last_)
	{
		send(ProtobufCodec(&msg), type, last_);
	}

	// async publish protobuf.
	inline void publish(
		IN const google::protobuf::Message& msg,
		IN int type, IN char snap_)
	{
		send(ProtobufCodec(&msg), type, snap_);
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
	SessionData::wptr	m_session_wptr;		// session data.
	SessionId			m_session_id;		// session id.
	
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
	inline TcpConnection& connection() const;

	// get session data.
	inline const SessionId id() const;

	// get session data.
	inline SessionData::ptr data() const;

	// get and cast session data.
	template<typename SessionDataT>
	inline std::shared_ptr<SessionDataT> cast();

public:
	// async response message.
	inline void response(IN MessageMeta& meta, IN const Context& c);

	// async send message.
	inline void send(IN const MessageMeta& meta);

	// async send authority info.
	inline void authorize(IN const MessageMeta& meta);

#ifndef ECO_NO_PROTOBUF
	// async send protobuf.
	inline void send(
		IN const google::protobuf::Message& msg,
		IN int type, IN bool last_)
	{
		ProtobufCodec cdc(&msg);
		send(MessageMeta().codec(cdc).message_type(type).last(last_));
	}

	// async send protobuf authority info.
	inline void authorize(
		IN const google::protobuf::Message& msg,
		IN int type, IN bool last_)
	{
		ProtobufCodec cdc(&msg);
		send(MessageMeta().codec(cdc).message_type(type).last(last_));
	}

	// async send response to client by context.
	inline void response(
		IN const google::protobuf::Message& msg,
		IN int type, IN bool last_, IN const Context& c)
	{
		ProtobufCodec cdc(&msg);
		response(MessageMeta().codec(cdc).message_type(type).last(last_), c);
	}
#endif

private:
	// implement.
	mutable TcpSessionImpl m_impl;
	friend class TcpSessionOuter;
};
}}

#include <eco/net/TcpSession.inl>
////////////////////////////////////////////////////////////////////////////////
#endif