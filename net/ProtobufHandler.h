#ifndef ECO_NET_PROTOBUF_HANDLER_H
#define ECO_NET_PROTOBUF_HANDLER_H
#ifndef ECO_NO_PROTOBUF
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-17.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/net/RequestHandler.h>
#include <eco/net/protocol/ProtobufCodec.h>
#include <eco/sys/Win.h>



namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
template<typename message_t, typename handler_t>
class ProtobufHandler : public RequestHandler<message_t, handler_t>
{
public:
	typedef ProtobufHandler<message_t, handler_t> Handler;

	inline bool on_decode(IN const char* bytes, IN uint32_t size)
	{
		ProtobufCodec codec(request());
		return codec.decode(bytes, size);
	}

public:
	// response message to the request.
	inline void response(
		IN const google::protobuf::Message& msg,
		IN MessageOption& opt)
	{
		context().response(&ProtobufCodec(msg), opt);
	}

	// response message to the request.
	inline void response(
		IN const google::protobuf::Message& msg,
		IN const bool is_last = true)
	{
		response_type(msg, get_response_type(), is_last);
	}

	// response message with no object.
	inline void response()
	{
		response_error(0);
	}

	// response message to the request.
	inline void response_type(
		IN const google::protobuf::Message& msg,
		IN const uint32_t msg_type,
		IN const bool is_last = true)
	{
		MessageOption opt(msg_type, 0, is_last);
		context().response(&ProtobufCodec(msg), opt);
	}

	// response message to the request.
	inline void response_error(
		IN const google::protobuf::Message& msg,
		IN const uint32_t error_id)
	{
		MessageOption opt(get_response_type(), error_id, true);
		context().response(&ProtobufCodec(msg), opt);
	}
	inline void response_error(IN const uint32_t error_id)
	{
		MessageOption opt(get_response_type(), error_id, true);
		context().response(nullptr, opt);
	}

public:
	// response message to the request.
	inline static void async_response(
		IN const google::protobuf::Message& msg,
		IN const uint32_t async_id,
		IN MessageOption& opt,
		IN const bool logging = true)
	{
		auto h = pop_async(async_id, opt.is_last());
		if (h == 0 && logging)
		{
			ECO_WARN << eco::net::Log(nullptr, handler_t::response_type(),
				handler_t::name())(eco::net::rsp);
			return;
		}
		h->response(msg, opt);
		if (logging) ECO_HDL(debug, *h);
	}

	// response message to the request.
	inline static void async_response(
		IN const google::protobuf::Message& msg,
		IN const uint32_t async_id,
		IN const bool is_last = true,
		IN const bool logging = true)
	{
		uint32_t type = handler_t::response_type();
		async_response_type(msg, async_id, type, is_last, logging);
	}

	// response message to the request.
	inline static void async_response_type(
		IN const google::protobuf::Message& msg,
		IN const uint32_t async_id,
		IN const uint32_t msg_type,
		IN const bool is_last,
		IN const bool logging = true)
	{
		MessageOption opt(msg_type, 0, is_last);
		async_response(msg, async_id, opt, logging);
	}

	// response message to the request.
	inline void async_response_error(
		IN const google::protobuf::Message& msg,
		IN const uint32_t async_id,
		IN const uint32_t error_id)
	{
		MessageOption opt(handler_t::response_type(), error_id, true);
		async_response(msg, async_id, opt, true);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif
#endif