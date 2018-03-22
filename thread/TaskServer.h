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
class Task : public eco::Object<Task>
{
public:
	typedef std::auto_ptr<Task> aptr;
	virtual ~Task() {};
	virtual void operator()(void) = 0;
};


////////////////////////////////////////////////////////////////////////////////
class TaskHandler
{
public:
	template<typename TaskT>
	inline void operator()(IN TaskT& task)
	{
		task();
	}

	template<typename TaskT>
	inline void operator()(IN std::auto_ptr<TaskT>& task)
	{
		(*task)();
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
template<typename TaskT = Task::aptr>
class TaskServer : public MessageServer<TaskT, TaskHandler>
{
public:
};


////////////////////////////////////////////////////////////////////////////////
}
#endif