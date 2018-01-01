#ifndef ECO_NET_DISPATCH_SERVER_H
#define ECO_NET_DISPATCH_SERVER_H
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
#include <eco/net/Ecode.h>
#include <eco/net/Context.h>
#include <eco/net/DispatchRegistry.h>
#include <eco/thread/DispatchServer.h>



namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class DispatchHandler : public eco::DispatchHandler<uint64_t, Context>
{
public:
	/*@ dispatch message to message handler.
	* @ para.dc: message to be dispatched.
	*/
	void operator()(IN DataContext& dc) const;
};


////////////////////////////////////////////////////////////////////////////////
class DispatchServer :
	public eco::net::DispatchRegistry,
	public eco::MessageServer<DataContext, DispatchHandler>
{
public:
	virtual void register_handler(
		IN const uint64_t id,
		IN HandlerFunc hf) override
	{
		message_handler().set_dispatch(id, hf);
	}

	virtual void register_default_handler(IN HandlerFunc hf) override
	{
		message_handler().set_default(hf);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif