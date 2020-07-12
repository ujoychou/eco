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
#include <eco/net/TcpPeer.h>
#include <eco/net/protocol/ProtobufCodec.h>


namespace eco{;
namespace net{;
class DataContext;
////////////////////////////////////////////////////////////////////////////////
// noncopyable conn data, only can be used in local.
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

	inline TcpPeer::ptr peer()
	{
		return m_peer;
	}

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

	inline TcpConnection(TcpPeer::wptr& wptr, Protocol& prot, size_t id)
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
			eco::Error e(e_peer_server_close, "server close this peer.");
			peer->close_and_notify(e);
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
	inline bool authorized() const;

	// get tcp connection id.
	inline ConnectionId get_id() const
	{
		return m_id;
	}

	// get tcp connection remote client ip.
	inline eco::String get_ip() const
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer == nullptr)
		{
			ECO_THROW(e_peer_expired)
				<< "get_ip fail peer has expired, it has been closed.";
		}
		return std::move(peer->get_ip());
	}

	template<typename ConnectionDataT>
	inline ConnectionDataPtr<ConnectionDataT> cast() const
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer == nullptr)
		{
			ECO_THROW(e_peer_expired)
				<< "cast data fail peer has expired, it has been closed.";
		}
		return ConnectionDataPtr<ConnectionDataT>(peer);
	}

public:
	// send string message.
	inline void send(IN eco::String& data, IN uint32_t start)
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer != nullptr)
		{
			peer->send(data, start);
		}
	}

	// send message.
	inline void send(IN const MessageMeta& meta)
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer != nullptr)
		{
			peer->send(meta, *m_prot);
		}
	}

	inline void send(IN Codec* codec, IN MessageOption& opt)
	{
		send(MessageMeta(codec, opt));
	}

	inline void send(IN Codec* codec, IN uint32_t type, IN bool is_last)
	{
		send(codec, MessageOption(type, 0, is_last));
	}

	// response message.
	inline void response(IN Codec* codec, MessageOption& opt, const Context& c)
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer != nullptr)
		{
			peer->response(MessageMeta(codec, opt), *m_prot, c);
		}
	}

	// async response message.
	inline void response(
		IN Codec* codec,
		IN uint32_t msg_type,
		IN uint32_t error_id,
		IN bool	is_last,
		IN const Context& context)
	{
		response(codec, MessageOption(msg_type, error_id, is_last), context);
	}

#ifndef ECO_NO_PROTOBUF
	// async send protobuf.
	inline void send(
		IN const google::protobuf::Message& msg,
		IN MessageOption& opt)
	{
		send(&ProtobufCodec(msg), opt);
	}

	// async send protobuf.
	inline void send(
		IN const google::protobuf::Message& msg,
		IN uint32_t msg_type,
		IN bool is_last)
	{
		send(&ProtobufCodec(msg), msg_type, is_last);
	}

	// async send response by context.
	inline void response(
		IN const google::protobuf::Message& msg,
		IN MessageOption& opt,
		IN const Context& context)
	{
		response(&ProtobufCodec(msg), opt, context);
	}

	// async send response by context.
	inline void response(
		IN const google::protobuf::Message& msg,
		IN uint32_t msg_type,
		IN uint32_t error_id,
		IN bool is_last,
		IN const Context& context)
	{
		response(&ProtobufCodec(msg), msg_type, error_id, is_last, context);
	}

	// async publish protobuf.
	inline void publish(
		IN const google::protobuf::Message& msg,
		IN MessageOption& opt)
	{
		send(&ProtobufCodec(msg), opt);
	}

	// async publish protobuf.
	inline void publish(
		IN const google::protobuf::Message& msg,
		IN uint32_t msg_type,
		IN uint8_t  snap_meta)
	{
		send(&ProtobufCodec(msg), MessageOption(msg_type).snap(snap_meta));
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
	virtual bool authorized() const
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
inline bool TcpConnection::authorized() const
{
	TcpPeer::ptr peer = m_peer.lock();
	return peer != nullptr && peer->data() != nullptr
		&& peer->data()->authorized();
}

// default connection factory function.
template<typename ConnectionDataT>
inline static ConnectionData* make_connection_data()
{
	return new ConnectionDataT();
}
// set connection factory to create connection of tcp server peer.
typedef ConnectionData* (*MakeConnectionDataFunc)();


////////////////////////////////////////////////////////////////////////////////
// tcp client on connect/close event.
typedef std::function<void(const eco::Error* e)> ClientOpenFunc;
typedef std::function<void(const eco::Error& e)> ClientCloseFunc;

// tcp server on accept/close tcp connection(tcp peer) event.
typedef std::function<void(ConnectionId id)> ServerAcceptFunc;
typedef std::function<void(ConnectionId id)> ServerCloseFunc;
typedef std::function<void(TcpPeer::ptr&, DataContext&)> OnRecvDataFunc;
typedef std::function<bool(MessageHead&, const char*, uint32_t)>
OnDecodeHeadFunc;


////////////////////////////////////////////////////////////////////////////////
}}
#endif