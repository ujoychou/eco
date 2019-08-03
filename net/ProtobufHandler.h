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
#include <eco/proxy/Win.h>



namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
template<typename message_t, typename handler_t>
class ProtobufHandler : public RequestHandler<message_t, handler_t>
{
public:
	typedef ProtobufHandler<message_t, handler_t> Handler;

	inline bool on_decode(
		IN const char* bytes,
		IN const uint32_t size)
	{
		ProtobufCodec codec(request());
		return codec.decode(bytes, size);
	}

	// response message to the request.
	inline void response(
		IN google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const bool last = true,
		IN const bool encrypted = false)
	{
		context().response(ProtobufCodec(msg), type, last, encrypted);
	}

	// response message to the request.
	inline void response(
		IN google::protobuf::Message& msg,
		IN const bool last = true,
		IN const bool encrypted = false)
	{
		auto type = get_response_type();
		context().response(ProtobufCodec(msg), type, last, encrypted);
	}

	// response message to the request.
	inline static void async_message(
		IN google::protobuf::Message& msg,
		IN uint32_t req_id,
		IN const bool last = true,
		IN const bool encrypted = false,
		IN const bool logging = true)
	{
		auto h = pop_async(req_id, last);
		if (h == 0 && logging)
		{
			ECO_LOG(warn)(eco::net::rsp) << eco::net::Log(
				nullptr, handler_t::response_type(), handler_t::name());
			return;
		}
		h->response(msg, last, encrypted);
		if (logging) ECO_HDL(debug, *h);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif
#endif