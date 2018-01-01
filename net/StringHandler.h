#ifndef ECO_NET_STRING_HANDLER_H
#define ECO_NET_STRING_HANDLER_H
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
#include <eco/net/protocol/StringCodec.h>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class StringHandler : public RequestHandler<std::string>
{
public:
	virtual bool on_decode(
		IN const char* bytes,
		IN const uint32_t size) override
	{
		return true;
	}

	inline void async_resp(
		IN const std::string& msg,
		IN uint16_t iid,
		IN bool is_last = true,
		IN MessageCategory category = category_general)
	{
		//StringCodec codec(msg);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif