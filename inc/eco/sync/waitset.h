#pragma once
/*******************************************************************************
@ name
waitset

@ function
waiting task done, fail, timeout.

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/macro.hpp>
#include <mutex>
#include <condition_variable>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
class waitset
{
public:
	inline explicit waitset(int32_t task_count = 1) : m_task_count(task_count)
	{}

	inline std::mutex& mutex()
	{
		return m_mutex;
	}

	inline void reset(int32_t task_count = 1)
	{
		std::lock_guard<std::mutex> lock(mutex());
		m_task_count = task_count;
	}

	inline bool wait()
	{
		std::unique_lock<std::mutex> lock(mutex());
		if (m_task_count > 0)
		{
			m_cond_var.wait(lock);
		}
		return (m_task_count == 0);
	}

	/*@ coordinator timed wait tasks finish.
	* @ return: if tasks finised: ok; if tasks fail: return fail; else return
	timeout.
	*/
	inline int timed_wait(int32_t millsec)
	{
		std::unique_lock<std::mutex> lock(mutex());
		if (m_task_count > 0)
		{
			if (std::cv_status::timeout == m_cond_var.wait_for(
				lock, std::chrono::milliseconds(millsec)))
			{
				return -1;
			}
		}
		return (m_task_count == 0) ? 1 : 0;
	}

	// finish all task.
	inline void done()
	{
		std::lock_guard<std::mutex> lock(mutex());
		m_task_count = 0;
		m_cond_var.notify_all();
	}

	// finish one task.
	inline void one()
	{
		std::lock_guard<std::mutex> lock(mutex());
		--m_task_count;
		if (m_task_count == 0)
		{
			m_cond_var.notify_all();
		}
	}

	inline void fail()
	{
		std::lock_guard<std::mutex> lock(mutex());
		m_task_count = -1;
		m_cond_var.notify_all();
	}

private:
	int32_t m_task_count;
	mutable std::mutex m_mutex;
	std::condition_variable m_cond_var;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);
