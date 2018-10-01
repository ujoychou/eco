#ifndef ECO_THREAD_EXCLUSIVE_MONITOR_H
#define ECO_THREAD_EXCLUSIVE_MONITOR_H
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2017 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/thread/ExclusiveWorker.h>
#include <eco/thread/ConditionVariable.h>


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
class ExclusiveMonitor : public ExclusiveWorker
{
public:
	inline ExclusiveMonitor()
	{}

	inline void wait_occupy()
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		while (busy())
		{
			m_cond_var.wait();
		}
		m_busy = 1;
	}

	inline bool timed_occupy(IN uint32_t millsec)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		while (busy())
		{
			if (!m_cond_var.timed_wait(millsec))
			{
				return false;
			}
		}
		m_busy = 1;
		return true;
	}

	inline void release()
	{
		ExclusiveWorker::release();
		m_cond_var.notify_one();
	}

protected:
	eco::ConditionVariable m_cond_var;
};


////////////////////////////////////////////////////////////////////////////////
class ExclusiveMonitorInit : public ExclusiveWorkerInit
{
public:
	inline ExclusiveMonitorInit(
		IN ExclusiveMonitor& em)
		: ExclusiveWorkerInit(em)
	{}

	inline ~ExclusiveMonitorInit()
	{
		if (m_worker_occupy && !m_task_state)
		{
			monitor().release();
		}
	}

	inline ExclusiveMonitor& monitor()
	{
		return (ExclusiveMonitor&)(m_worker);
	}

	// occupy the task.
	inline void wait_occupy()
	{
		monitor().wait_occupy();
	}

	// occupy the task.
	inline bool timed_occupy(IN uint32_t millsec)
	{
		return monitor().timed_occupy(millsec);
	}
};


////////////////////////////////////////////////////////////////////////////////
class ExclusiveMonitorExit
{
public:
	inline ExclusiveMonitorExit(
		IN ExclusiveMonitor& em,
		IN uint32_t is_exit = true)
		: m_monitor(em)
		, m_is_exit(is_exit)
	{}

	inline ~ExclusiveMonitorExit()
	{
		if (m_is_exit)
		{
			m_monitor.release();
		}
	}

private:
	ExclusiveMonitor& m_monitor;
	uint32_t m_is_exit;
};


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif