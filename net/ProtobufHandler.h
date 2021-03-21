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
#include <eco/App.h>
#include <eco/net/RequestHandler.h>
#include <eco/net/protocol/ProtobufCodec.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
template<typename message_t, typename handler_t>
class ProtobufHandler : public RequestHandler<message_t, handler_t>
{
public:
	typedef ProtobufHandler<message_t, handler_t> Handler;

	inline bool on_decode(IN const char* bytes, IN uint32_t size)
	{
		ProtobufCodec codec(&request());
		return codec.decode(bytes, size) != nullptr;
	}

	inline void to_request()
	{
		if (handler_t::req_sev() == 0) return;
		ECO_REQ_SEV(handler_t::req_sev()) << eco::net::to_json(request());
	}
};


////////////////////////////////////////////////////////////////////////////////
class GetLocaleHandler : 
	public eco::net::ProtobufHandler<NullRequest, GetLocaleHandler>
{
	ECO_HANDLE_OPTION(auth_none);
	ECO_HANDLE_LOG(none, none, none);	// disable logging locale.
	ECO_HANDLE(eco::proto_locale_get_req, eco::proto_locale_get_rsp, "locale");
public:
	inline void on_request()
	{
		ECO_REQ(debug);
		resolve(&eco::App::get()->locale().data());
		ECO_RSP(debug);
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#endif
#endif