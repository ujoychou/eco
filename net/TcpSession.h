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
#include <eco/net/protocol/Protocol.h>
#include <eco/net/protocol/ProtobufCodec.h>
#include <memory>

namespace eco{;
namespace net{;
class TcpPeer;
class TcpClient;
class Context;
////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpSession
{
	ECO_SHARED_API(TcpSession)
public:
	// set peer host, client or server.
	void set_host(IN TcpSessionHost& host);
	void set_protocol(IN Protocol& prot);

	// if a none session open a new session, else get the exist session.
	bool open(IN const SessionId session_id = none_session);

	// check whether session has been opened.
	bool opened() const;

	// close session, release session data.
	void close();

	// get session data.
	uint32_t get_session_id() const;

	// get session data.
	SessionData::ptr data();

	// get and cast session data.
	template<typename SessionDataT>
	inline std::shared_ptr<SessionDataT> cast()
	{
		return std::dynamic_pointer_cast<SessionDataT>(data());
	}

	// get session peer.
	TcpPeer& get_peer() const;

public:
	// async send message.
	void async_send(IN MessageMeta& meta);

	// async send authority info.
	void async_auth(IN MessageMeta& meta);

	// async response message.
	void async_resp(
		IN Codec& codec,
		IN const uint32_t type,
		IN const Context& context,
		IN const bool last = false);

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
		MessageMeta meta(codec, get_session_id(), type, category);
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
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif