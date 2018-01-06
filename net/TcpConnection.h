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
#include <eco/net/SessionData.h>
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
	ECO_PTR_MEMBER(ConnectionDataT,
		static_cast<ConnectionDataT*>(m_peer->data()));
public:
	inline ConnectionDataPtr(IN TcpPeer::ptr& peer) : m_peer(peer)
	{}

	inline ConnectionDataPtr(IN ConnectionDataPtr&& v)
		: m_peer(std::move(v.peer))
	{}

private:
	TcpPeer::ptr m_peer;
};

////////////////////////////////////////////////////////////////////////////////
class TcpConnection
{
public:
	inline TcpConnection() : m_protocol(nullptr)
	{}

	inline TcpConnection(IN TcpPeer::wptr& wptr, IN Protocol& prot)
		: m_peer_wptr(wptr), m_protocol(&prot)
	{}

	inline TcpConnection(IN const TcpConnection& other) :
		m_peer_wptr(other.m_peer_wptr),
		m_protocol(other.m_protocol)
	{}

	// close session, release session data.
	inline void close()
	{
		TcpPeer::ptr peer = m_peer_wptr.lock();
		if (peer != nullptr)
		{
			peer->notify_close(nullptr);
		}
	}

	inline void clear()
	{
		m_peer_wptr.reset();
		m_protocol = nullptr;
	}

	// check whether this connection has been expired.
	inline bool expired() const
	{
		return m_peer_wptr.expired();
	}

	// check whether this connection has been authed.
	inline bool authed() const;

	// get session data.
	inline uint64_t get_id() const
	{
		TcpPeer::ptr peer = m_peer_wptr.lock();
		if (peer != nullptr)
		{
			return peer->get_id();
		}
		return 0;
	}

	template<typename ConnectionDataT>
	inline ConnectionDataPtr<ConnectionDataT> cast() const
	{
		TcpPeer::ptr peer = m_peer_wptr.lock();
		if (peer == nullptr)
		{
			EcoThrow(e_peer_expired)
				<< "peer has expired, it has been closed.";
		}
		return ConnectionDataPtr<ConnectionDataT>(peer);
	}

public:
	// async response message.
	inline void async_resp(
		IN Codec& codec,
		IN const uint32_t type,
		IN const Context& context,
		IN const bool last = true)
	{
		TcpPeer::ptr peer = m_peer_wptr.lock();
		if (peer != nullptr)
		{
			return peer->async_resp(codec, type, context, *m_protocol);
		}
	}

	// async send protobuf.
	inline void async_send(
		IN google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const MessageCategory category = category_message)
	{
		ProtobufCodec codec(msg);
		MessageMeta meta(codec, none_session, type, category);
		async_send(meta);
	}

	// async send response to client by context.
	inline void async_resp(
		IN google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const Context& context,
		IN const bool last = true)
	{
		ProtobufCodec codec(msg);
		async_resp(codec, type, context, last);
	}

private:
	// async send message.
	inline void async_send(IN MessageMeta& meta)
	{
		TcpPeer::ptr peer = m_peer_wptr.lock();
		if (peer != nullptr)
		{
			return peer->async_send(meta, *m_protocol);
		}
	}

	TcpPeer::wptr	m_peer_wptr;
	Protocol*		m_protocol;
};


////////////////////////////////////////////////////////////////////////////////
// define session data holder. (using the boost::any<type> mechanism)
class ConnectionData : public eco::HeapOperators
{
	ECO_OBJECT(ConnectionData);
public:
	inline  ConnectionData() : m_prot(nullptr) {}
	virtual ~ConnectionData() {}

	// get connection object.
	inline TcpConnection connection()
	{
		return TcpConnection(m_wptr, *m_prot);
	}

	// whether this connection has authorized.
	virtual bool authed() const
	{
		return false;
	}

private:
	TcpPeer::wptr m_wptr;
	Protocol* m_prot;
	friend class TcpPeer;
};

// default session factory function.
template<typename ConnectionDataT>
inline static ConnectionData* make_connection_data()
{
	return new ConnectionDataT();
}

// set session factory to create session of tcp server peer.
typedef ConnectionData* (*MakeConnectionDataFunc)();


////////////////////////////////////////////////////////////////////////////////
inline bool TcpConnection::authed() const
{
	TcpPeer::ptr peer = m_peer_wptr.lock();
	return peer != nullptr && peer->data() != nullptr
		&& peer->data()->authed();
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif