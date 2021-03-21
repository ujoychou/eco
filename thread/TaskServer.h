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
#include <eco/thread/Btask.h>
#include <eco/thread/MessageServer.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class TaskHandler
{
public:
	// set post func.
	typedef std::function<void(const Closure&)> PostFunc;
	inline void set_post(PostFunc&& func)
	{
		m_post_func = func;
	}

	// restart seconds if fail.
	inline void operator()(const Closure& task, int restart_sec_if_fail = 0)
	{
		try
		{
			task();
		}
		catch (std::exception& e)
		{
			ECO_LOG(error, "task") < e.what();
			if (m_post_func && restart_sec_if_fail)
			{
				Eco::get().timer().run_after(
					[=]() mutable {	m_post_func(std::move(task)); },
					restart_sec_if_fail * 1000, false);
			}
		}// end try/catch;
	}

private:
	PostFunc m_post_func;
};


////////////////////////////////////////////////////////////////////////////////
class TaskServer : public MessagePool<Closure, TaskHandler>
{
public:
	inline TaskServer()
	{
		m_message_handler.set_post(std::bind(
			&TaskServer::post, this, std::placeholders::_1));
	}

	inline void post(const Closure& task)
	{
		T::post(task);
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif