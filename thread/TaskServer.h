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
	inline void operator()(IN Task::ptr& task)
	{
		// execute this task.
		bool result = true;
		try
		{
			(*task)();
		}
		catch (std::exception& e)
		{
			result = false;
			ECO_LOGX(error) << task->get_type_name() << ": " << e.what();
		}

		if (!result && m_post_func)
		{
			eco().timer().add(task->restart_secs() * 1000, false,
				std::bind(m_post_func, task));
			return;
		}
	}

	// set post func
	typedef std::function<void(Task::ptr&)> PostFunc;
	inline void set_post(PostFunc func)
	{
		m_post_func = func;
	}

private:
	PostFunc m_post_func;
};


////////////////////////////////////////////////////////////////////////////////
class TaskServer : public MessageServer<Task::ptr, TaskHandler>
{
public:
	inline TaskServer()
	{
		m_message_handler.set_post(
			std::bind(&T::post, this, std::placeholders::_1));
	}

	typedef MessageServer<Task::ptr, TaskHandler> super;
	inline void post(IN std::function<void(void)> task)
	{
		super::post(Task::ptr(new FuncTask(task)));
	}

	inline void post(IN Task::ptr& task)
	{
		super::post(task);
	}
};


////////////////////////////////////////////////////////////////////////////////
}
#endif