#ifndef ECO_NET_TCP_CONNECTION_H
#define ECO_NET_TCP_CONNECTION_H
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
#include <eco/net/protocol/Protocol.h>
#include <eco/net/protocol/ProtobufCodec.h>
#include <eco/net/TcpPeer.h>
#include <memory>

namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
template<typename ConnectionDataT>
class ConnectionDataPtr
{
	ECO_OBJECT(ConnectionDataPtr);
	ECO_PTR_MEMBER(ConnectionDataT, m_data);
public:
	inline ConnectionDataPtr(IN TcpPeer::ptr& peer)
		: m_peer(peer), m_data(static_cast<ConnectionDataT*>(m_peer->data()))
	{}

	inline ConnectionDataPtr(IN ConnectionDataPtr&& v)
		: m_peer(std::move(v.peer))
		, m_data(static_cast<ConnectionDataT*>(v.m_data))
	{}

private:
	TcpPeer::ptr m_peer;
	ConnectionDataT* m_data;
};

////////////////////////////////////////////////////////////////////////////////
class TcpConnection
{
public:
	inline TcpConnection() : m_prot(nullptr), m_id(0)
	{}

	inline TcpConnection(
		IN TcpPeer::wptr& wptr, 
		IN Protocol& prot,
		IN const size_t id)
		: m_peer(wptr), m_prot(&prot), m_id(id)
	{}

	inline TcpConnection(IN const TcpConnection& other)
		: m_peer(other.m_peer), m_prot(other.m_prot), m_id(other.m_id)
	{}

	inline void move(IN TcpConnection& other)
	{
		m_peer = std::move(other.m_peer);
		m_prot = other.m_prot;
		m_id = other.m_id;
	}

	// close session, release session data.
	inline void close()
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer != nullptr)
		{
			peer->close_and_notify(nullptr);
		}
	}

	inline void clear()
	{
		m_peer.reset();
		m_prot = nullptr;
		m_id = 0;
	}

	// check whether this connection has been expired.
	inline bool expired() const
	{
		return m_peer.expired();
	}

	// check whether this connection has been authed.
	inline bool authed() const;

	// get tcp connection id.
	inline ConnectionId get_id() const
	{
		return m_id;
	}

	// get tcp connection remote client ip.
	inline const eco::String get_ip() const
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer == nullptr)
		{
			EcoThrow(e_peer_expired)
				<< "get_ip fail peer has expired, it has been closed.";
		}
		return (eco::String&&)peer->get_ip();
	}

	template<typename ConnectionDataT>
	inline ConnectionDataPtr<ConnectionDataT> cast() const
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer == nullptr)
		{
			EcoThrow(e_peer_expired)
				<< "cast data fail peer has expired, it has been closed.";
		}
		return ConnectionDataPtr<ConnectionDataT>(peer);
	}

public:
	// async response message.
	inline void async_response(
		IN Codec& codec,
		IN const uint32_t type,
		IN const Context& context,
		IN const bool encrypted = true,
		IN const bool last = true)
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer != nullptr)
		{
			return peer->async_response(
				codec, type, context, *m_prot, encrypted, last);
		}
	}

	// async send message.
	inline void async_send(IN const MessageMeta& meta)
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer != nullptr)
		{
			return peer->async_send(meta, *m_prot);
		}
	}

#ifndef ECO_NO_PROTOBUF
	// async send protobuf.
	inline void async_send(
		IN const google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const bool encrypted = true)
	{
		ProtobufCodec codec(msg);
		MessageMeta meta(codec, none_session, type, encrypted);
		async_send(meta);
	}

	// async send response to client by context.
	inline void async_response(
		IN const google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const Context& context,
		IN const bool encrypted = true,
		IN const bool last = true)
	{
		ProtobufCodec codec(msg);
		async_response(codec, type, context, encrypted, last);
	}

	// async send protobuf.
	inline void async_send_session(
		IN const google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const SessionId sess_id,
		IN const bool encrypted = true)
	{
		ProtobufCodec codec(msg);
		MessageMeta meta(codec, sess_id, type, encrypted);
		async_send(meta);
	}
#endif

private:
	TcpPeer::wptr	m_peer;
	Protocol*		m_prot;
	size_t			m_id;
	friend class TcpConnectionOuter;
};


////////////////////////////////////////////////////////////////////// TcpServer
class ConnectionData : public eco::HeapOperators
{
	ECO_OBJECT(ConnectionData);
public:
	// #.event: after server connection connect.
	inline ConnectionData() {}

	// #.event: after server connection close.
	virtual ~ConnectionData() = 0 {}

	// whether this connection has authorized.
	virtual bool authed() const
	{
		return false;
	}

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

	inline const ConnectionId get_id() const
	{
		return m_conn.get_id();
	}

private:
	TcpConnection m_conn;
};


////////////////////////////////////////////////////////////////////////////////
inline bool TcpConnection::authed() const
{
	TcpPeer::ptr peer = m_peer.lock();
	return peer != nullptr && peer->data() != nullptr
		&& peer->data()->authed();
}

// default connection factory function.
template<typename ConnectionDataT>
inline static ConnectionData* make_connection_data()
{
	return new ConnectionDataT();
}
// set connection factory to create connection of tcp server peer.
typedef ConnectionData* (*MakeConnectionDataFunc)();


////////////////////////////////////////////////////////////////////// TcpClient
typedef void (*OnConnectFunc) ();		// tcp client on connect event.
typedef void (*OnCloseFunc) ();			// tcp client on close event.


////////////////////////////////////////////////////////////////////////////////
}}
#endif