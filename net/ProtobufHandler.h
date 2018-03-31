#ifndef ECO_NET_PROTOBUF_HANDLER_H
#define ECO_NET_PROTOBUF_HANDLER_H
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



namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
template<typename ProtobufMessage>
class ProtobufHandler : public RequestHandler<ProtobufMessage>
{
public:
	typedef ProtobufHandler<ProtobufMessage> Handler;

	inline bool on_decode(
		IN const char* bytes,
		IN const uint32_t size)
	{
		ProtobufCodec codec(request());
		return codec.decode(bytes, size);
	}

	// response message to the request.
	inline void async_response(
		IN google::protobuf::Message& msg,
		IN uint32_t type = 0,
		IN const bool last = true,
		IN const bool encrypted = true)
	{
		if (type == 0) type = get_response_type();
		context().async_response(ProtobufCodec(msg), type, last, encrypted);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif