#ifndef ECO_THREAD_MONITOR_H
#define ECO_THREAD_MONITOR_H
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
#include <eco/std/mutex.h>
#include <eco/std/condition_variable.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class Monitor : public eco::Object<Monitor>
{
public:
	/*@ init coordinator with tasks total number, when all tasks have been
	finished, coordinator will notify waiter ok, else fail.
	*/
	inline explicit Monitor(IN int32_t task_count = 1)
		: m_task_count(task_count)
	{}

	/*@ reset monitor to wait.*/
	inline std_mutex& mutex()
	{
		return m_mutex;
	}

	/*@ reset monitor to wait.*/
	inline void reset(IN int32_t task_count = 1)
	{
		std_lock_guard lock(mutex());
		m_task_count = task_count;
	}

	/*@ coordinator wait tasks finish.
	* @ return: tasks finised return true, else return false.
	*/
	inline bool wait()
	{
		std_lock_guard lock(mutex());
		if (m_task_count > 0)
		{
			m_cond_var.wait();
		}
		return (m_task_count == 0);
	}

	/*@ coordinator timed wait tasks finish.
	* @ return: if tasks finised: ok; if tasks fail: return fail; else return
	timeout.
	*/
	inline eco::Result timed_wait(IN int32_t millsec)
	{
		std_lock_guard lock(mutex());
		if (m_task_count > 0)
		{
			if (!m_cond_var.timed_wait(millsec))
			{
				return eco::timeout;
			}
		}
		return (m_task_count == 0) ? eco::ok : eco::fail;
	}

	// finish one task.
	inline void finish()
	{
		std_lock_guard lock(mutex());
		m_task_count = 0;
		m_cond_var.notify_all();
	}

	// finish one task.
	inline void finish_one()
	{
		std_lock_guard lock(mutex());
		--m_task_count;
		if (m_task_count == 0)
		{
			m_cond_var.notify_all();
		}
	}

	// one task failed.
	inline void fail()
	{
		std_lock_guard lock(mutex());
		m_task_count = -1;
		m_cond_var.notify_all();
	}

private:
	int32_t m_task_count;
	mutable std_mutex m_mutex;
	std_condition_variable m_cond_var;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif