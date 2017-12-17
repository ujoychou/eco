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
	inline bool on_decode(
		IN const char* bytes,
		IN const uint32_t size)
	{
		ProtobufCodec codec(request());
		return codec.decode(bytes, size);
	}

	inline void response(
		IN const google::protobuf::Message& msg,
		IN uint16_t iid,
		IN bool is_last = true,
		IN MessageCategory category = category_general)
	{
		ProtobufCodec codec(const_cast<google::protobuf::Message&>(msg));
		Handler::response(codec, iid, is_last, category);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif