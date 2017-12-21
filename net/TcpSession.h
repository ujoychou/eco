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

public:
	// async send message.
	void async_send(
		IN Codec& codec,
		IN const uint32_t type,
		IN const MessageModel model,
		IN const MessageCategory category = category_message);

	// async send message.
	template<typename Codec, typename MessageT>
	inline void async_send(
		IN const MessageT& msg,
		IN const uint32_t type,
		IN const MessageModel model,
		IN const MessageCategory category = category_message)
	{
		return async_send(Codec(msg), type, model, category);
	}

	// async send message.
	inline void async_request(
		IN Codec& codec,
		IN const uint32_t type,
		IN const MessageCategory category = category_message)
	{
		return async_send(codec, type, model_req, category);
	}

	// async send message.
	template<typename Codec, typename MessageT>
	inline void async_request(
		IN const MessageT& msg,
		IN const uint32_t type,
		IN const MessageCategory category = category_message)
	{
		return async_send(Codec(msg), type, model_req, category);
	}

public:
	// async send protobuf.
	inline void async_send(
		IN google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const MessageModel model,
		IN const MessageCategory category = category_message)
	{
		return async_send(ProtobufCodec(msg), type, model, category);
	}

	// async send request protobuf.
	inline void async_request(
		IN google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const MessageCategory category = category_message)
	{
		return async_send(ProtobufCodec(msg), type, model_req, category);
	}

};


////////////////////////////////////////////////////////////////////////////////
}}
#endif