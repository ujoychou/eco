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
class TcpConnection
{
public:
	// check whether session has been opened.
	inline bool opened() const
	{
		return !m_peer_wptr.expired();
	}

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

	// get session data.
	inline ConnectionData::ptr data() const
	{
		TcpPeer::ptr peer = m_peer_wptr.lock();
		if (peer != nullptr)
		{
			return peer->data();
		}
		return ConnectionData::ptr();
	}

	// get and cast session data.
	template<typename SessionDataT>
	inline std::shared_ptr<SessionDataT> cast() const
	{
		TcpPeer::ptr peer = m_peer_wptr.lock();
		if (peer != nullptr)
		{
			return peer->cast<SessionDataT>();
		}
		return std::shared_ptr<SessionDataT>();
	}

public:
	// async send message.
	inline void async_send(IN MessageMeta& meta)
	{
		TcpPeer::ptr peer = m_peer_wptr.lock();
		if (peer != nullptr)
		{
			return peer->async_send(meta, *m_protocol);
		}
	}

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

	// async send message.
	template<typename Codec, typename MessageT>
	inline void async_send(
		IN const MessageT& msg,
		IN MessageMeta& meta)
	{
		Codec codec(msg);
		meta.m_codec = &codec;
		async_send(meta);
	}

	// async send protobuf.
	inline void async_send(
		IN google::protobuf::Message& msg,
		IN MessageMeta& meta)
	{
		ProtobufCodec codec(msg);
		meta.m_codec = &codec;
		async_send(meta);
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
	TcpPeer::wptr	m_peer_wptr;
	Protocol*		m_protocol;
	friend class DispatchHandler;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif