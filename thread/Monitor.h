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
#include <eco/thread/ConditionVariable.h>


namespace eco{;


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
	inline eco::Mutex& mutex()
	{
		return m_cond_var.mutex();
	}

	/*@ reset monitor to wait.*/
	inline void reset(IN int32_t task_count = 1)
	{
		m_task_count = task_count;
	}

	/*@ coordinator wait tasks finish.
	* @ return: tasks finised return true, else return false.
	*/
	inline bool wait()
	{
		eco::Mutex::ScopeLock lock(m_cond_var.mutex());
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
	inline int timed_wait(IN int32_t millsec)
	{
		eco::Mutex::ScopeLock lock(m_cond_var.mutex());
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
		eco::Mutex::ScopeLock lock(m_cond_var.mutex());
		m_task_count = 0;
		m_cond_var.notify_all();
	}

	// finish one task.
	inline void finish_one()
	{
		eco::Mutex::ScopeLock lock(m_cond_var.mutex());
		--m_task_count;
		if (m_task_count == 0)
		{
			m_cond_var.notify_all();
		}
	}

	// one task failed.
	inline void fail()
	{
		eco::Mutex::ScopeLock lock(m_cond_var.mutex());
		m_task_count = -1;
		m_cond_var.notify_all();
	}

private:
	int32_t m_task_count;
	eco::ConditionVariable m_cond_var;
};


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif