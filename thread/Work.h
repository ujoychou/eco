#ifndef ECO_THREAD_WORK_H
#define ECO_THREAD_WORK_H
/*******************************************************************************
@ name
message server.

@ function
1.asynchronous handle message.(thread size > 0)
2.synchronous handle message.(thread size = 0)


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Error.h>
#include <eco/log/Log.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<typename functor_t>
inline bool work_with_catch_exception(const char* name, functor_t&& func)
{
	bool result = false;
	eco::this_thread::dead_lock().begin();
	try
	{
		func();
		result = true;
	}
	catch(const eco::Error& e)
	{
		ECO_LOG(error, name) << e;
	}
	catch(const std::exception& e)
	{
		ECO_LOG(error, name) << e.what();
	}
	catch(...)
	{
		ECO_LOG(error, name) << eco::Error();
	}
	return result;
}


////////////////////////////////////////////////////////////////////////////////
template<typename functor_t>
inline bool work_with_catch_exception(functor_t&& func)
{
	return eco::work_with_catch_exception(this_thread::name(), std::move(func));
}
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif