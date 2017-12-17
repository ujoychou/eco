#ifndef ECO_THREAD_STATE_H
#define ECO_THREAD_STATE_H
/*******************************************************************************
@ name
thread safe object state.

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/thread/State.h>


namespace eco{;
namespace thread{;
////////////////////////////////////////////////////////////////////////////////
class State : public eco::atomic::State
{
public:
	inline void running()
	{
		add(_a);
	}
	inline void stop()
	{
		add(_b);
	}
	inline void stoped()
	{
		add(_c);
	}

	inline int is_prepare() const
	{
		return is_none();
	}
	inline int is_running() const
	{
		return has(_a);
	}
	inline int is_stopped() const
	{
		return has(_c);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}// ns::eco
#endif