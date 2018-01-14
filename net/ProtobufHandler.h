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
	inline void async_resp(
		IN google::protobuf::Message& msg,
		IN const uint32_t type,
		IN const bool last = true)
	{
		if (session().session_mode())
			session().async_resp(msg, type, m_context, last);
		else
			connection().async_resp(msg, type, m_context, last);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif