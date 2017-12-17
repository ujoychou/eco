#ifndef ECO_THREAD_TASK_SERVER_H
#define ECO_THREAD_TASK_SERVER_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Btask.h>
#include <eco/thread/MessageServer.h>


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
class TaskHandler
{
public:
	template<typename Task>
	inline void operator()(IN Task& task)
	{
		task();
	}

	inline void operator()(IN std::auto_ptr<Btask>& task)
	{
		(*task)();
	}
};


////////////////////////////////////////////////////////////////////////////////
class ClosureServer : public MessageServer<eco::Closure, TaskHandler>
{
public:
};


////////////////////////////////////////////////////////////////////////////////
template<typename Task>
class TaskServer : public MessageServer<Task, TaskHandler>
{
public:
};


////////////////////////////////////////////////////////////////////////////////
}
#endif