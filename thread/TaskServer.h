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
#include <eco/thread/Timer.h>
#include <eco/thread/Btask.h>
#include <eco/thread/MessageServer.h>


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
class TaskHandler
{
public:
	inline void operator()(IN TaskUnit& task)
	{
		// execute this task.
		bool result = true;
		try
		{
			task();
		}
		catch (std::exception& e)
		{
			result = false;
			ECO_LOG(error, "task") << task.get_type_name() << ": " << e.what();
		}

		if (!result && m_post_func)
		{
			Eco::get().timer().add(task.restart_secs() * 1000, false,
				[this, task] () mutable {
				m_post_func(std::move(task));
			});
			return;
		}
	}

	// set post func
	typedef std::function<void(TaskUnit&&)> PostFunc;
	inline void set_post(PostFunc&& func)
	{
		m_post_func = func;
	}

private:
	PostFunc m_post_func;
};


////////////////////////////////////////////////////////////////////////////////
class TaskServer : public MessagePool<TaskUnit, TaskHandler>
{
public:
	inline TaskServer()
	{
		void(T::*func)(TaskUnit&&) = &T::post;
		m_message_handler.set_post(
			std::bind(func, this, std::placeholders::_1));
	}

	inline void post(IN Closure&& task)
	{
		T::post(TaskUnit(std::forward<Closure>(task)));
	}

	inline void post(IN Task::ptr& task)
	{
		T::post(TaskUnit(task));
	}
	inline void post(IN Task::ptr&& task)
	{
		T::post(TaskUnit(task));
	}

private:
	inline void post(IN TaskUnit&& task)
	{
		T::post(std::forward<TaskUnit>(task));
	}
};


////////////////////////////////////////////////////////////////////////////////
struct ClosureHandler
{
	inline void operator()(IN Closure& task) {	task(); }
};
typedef MessageServerPool<Closure, ClosureHandler> ClosureServerPool;
typedef ClosureServerPool::MessageWorker ClosureWorker;


////////////////////////////////////////////////////////////////////////////////
}
#endif