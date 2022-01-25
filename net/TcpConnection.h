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
#include <eco/rx/RxApi.h>
#include <eco/net/TcpPeer.h>
#include <eco/net/protocol/ProtobufCodec.h>


ECO_NS_BEGIN(eco);
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
	inline TcpConnection() : m_id(0)
	{}

	inline TcpConnection(TcpPeer::wptr& wptr, size_t id_)
		: m_peer(wptr), m_id(id_)
	{}

	inline TcpConnection(IN const TcpConnection& other)
		: m_peer(other.m_peer), m_id(other.m_id)
	{}

	inline void move(IN TcpConnection& other)
	{
		m_peer = std::move(other.m_peer);
		m_id = other.m_id;
	}

	// close session, release session data.
	inline void close()
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer != nullptr)
		{
			ECO_THIS_ERROR(e_peer_server_close) < "server close this peer.";
			peer->close_and_notify();
		}
	}

	inline void clear()
	{
		m_peer.reset();
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
	inline const SessionId id() const
	{
		return m_id;
	}

	inline void authorize(const char* user, const char* lang)
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer) peer->authorize(user, lang, peer);
	}

	// get tcp connection remote client ip.
	inline eco::String ip() const
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer == nullptr)
		{
			ECO_THROW(e_peer_expired)
				< "get_ip fail peer has expired, it has been closed.";
		}
		return std::move(peer->ip());
	}

	// get user name.
	inline const char* user() const
	{
		TcpPeer::ptr peer = m_peer.lock();
		return peer ? peer->user() :  "";
	}
	inline const char* lang() const
	{
		TcpPeer::ptr peer = m_peer.lock();
		return peer ? peer->lang() : "";
	}

	// cast connection data.
	template<typename ConnectionDataT>
	inline ConnectionDataPtr<ConnectionDataT> cast() const
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer == nullptr)
		{
			ECO_THROW(e_peer_expired)
				< "cast data fail peer has expired, it has been closed.";
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
			peer->send(meta);
		}
	}

	inline void send(IN Codec& cdc, IN int type, IN bool is_last)
	{
		MessageMeta meta;
		meta.codec(cdc).message_type(type).last(is_last);
		send(meta);
	}

	// response message.
	inline void response(MessageMeta& meta, const Context& c)
	{
		TcpPeer::ptr peer = m_peer.lock();
		if (peer != nullptr)
		{
			peer->response(meta, c);
		}
	}

	// async response message.
	inline void response(Codec& cdc, int type, bool is_last, const Context& c)
	{
		MessageMeta meta;
		meta.codec(cdc).message_type(type).last(is_last);
		response(meta, c);
	}

#ifndef ECO_NO_PROTOBUF
	// async send protobuf.
	inline void send(
		IN const google::protobuf::Message& msg,
		IN int  type, IN bool last_)
	{
		send(ProtobufCodec(&msg), type, last_);
	}

	// async send response by context.
	inline void response(
		IN const google::protobuf::Message& msg,
		IN int type, bool last_, const Context& c)
	{
		response(ProtobufCodec(&msg), type, last_, c);
	}

	// async publish protobuf.
	inline void publish(
		IN const google::protobuf::Message& msg,
		IN int type, IN char snap_)
	{
		MessageMeta meta;
		ProtobufCodec cdc(&msg);
		meta.codec(cdc).message_type(type).snap(snap_);
		send(meta);
	}
#endif

private:
	TcpPeer::wptr	m_peer;
	SessionId		m_id;
	friend class TcpConnectionOuter;
};


////////////////////////////////////////////////////////////////////// TcpServer
class ConnectionData : public eco::RxHeap
{
	ECO_OBJECT(ConnectionData);
public:
	// #.event: after server connection connect.
	inline ConnectionData() {}

	// #.event: after server connection close.
	virtual ~ConnectionData() = 0 {}

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
		return m_conn.id();
	}

	inline const char* user() const
	{
		return m_conn.user();
	}

	inline const char* lang() const
	{
		return m_conn.lang();
	}

private:
	TcpConnection m_conn;
};


////////////////////////////////////////////////////////////////////////////////
inline bool TcpConnection::authorized() const
{
	TcpPeer::ptr peer = m_peer.lock();
	return peer != nullptr && peer->authorized();
}

// default connection factory function.
template<typename ConnectionDataT>
inline static ConnectionData* make_connection_data()
{
	return new ConnectionDataT();
}
// set connection factory to create connection of tcp server peer.
typedef ConnectionData* (*MakeConnectionData)();

////////////////////////////////////////////////////////////////////////////////
// tcp client on connect/close event.
typedef std::function<void(bool error)> OnConnect;
typedef std::function<void(void)> OnDisconnect;
// tcp client on load data event.
typedef std::function<void(void)> OnLoadFinish;
typedef std::function<void(OnLoadFinish)> OnLoadEvent;
typedef std::function<void(eco::atomic::State& state)> OnLoadState;

// tcp server on accept/close tcp connection(tcp peer) event.
typedef std::function<void(SessionId)> OnAccept;
typedef std::function<void(SessionId)> OnClose;
typedef std::function<void(TcpPeer::ptr&, DataContext&)> OnRecvData;
typedef std::function<eco::Result(MessageHead&, const char*, uint32_t)> OnDecodeHead;


////////////////////////////////////////////////////////////////////////////////
}}
#endif