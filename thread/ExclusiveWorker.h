#ifndef ECO_THREAD_EXCLUSIVE_WORKER_H
#define ECO_THREAD_EXCLUSIVE_WORKER_H
/*******************************************************************************
@ name

@ function
1.apply scene: client api(like ctp\esunny) control request once a time.
2.apply scene: all people just finish a task, who ocuppy first who do it.

@ exception

@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2017 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Project.h>
#include <eco/Thread/State.h>
#include <eco/thread/Mutex.h>


ECO_NS_BEGIN(eco);


////////////////////////////////////////////////////////////////////////////////
class ExclusiveWorker
{
public:
	inline ExclusiveWorker() : m_busy(0)
	{}

	inline eco::Mutex& mutex()
	{
		return m_mutex;
	}

	inline bool busy() const
	{
		return (m_busy == 1);
	}

	inline bool occupy()
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		if (busy())
		{
			return false;
		}
		m_busy = 1;
		return true;
	}

	inline void release()
	{
		m_busy = 0;
	}

protected:
	eco::Mutex m_mutex;
	eco::Atomic<uint32_t> m_busy;
};


////////////////////////////////////////////////////////////////////////////////
class ExclusiveWorkerInit
{
public:
	inline ExclusiveWorkerInit(
		IN ExclusiveWorker& worker)
		: m_worker(worker)
		, m_worker_occupy(false)
		, m_task_state(false)
	{}

	inline ~ExclusiveWorkerInit()
	{
		if (m_worker_occupy && !m_task_state)
		{
			m_worker.release();
		}
	}

	// occupy the task.
	inline bool occupy()
	{
		return m_worker_occupy = m_worker.occupy();
	}

	// finish the task.
	inline void set_task_state(IN bool is_ok = true)
	{
		m_task_state = is_ok;
	}

protected:
	ExclusiveWorker& m_worker;
	uint16_t m_worker_occupy;
	uint16_t m_task_state;
};


////////////////////////////////////////////////////////////////////////////////
class ExclusiveWorkerExit
{
public:
	inline ExclusiveWorkerExit(
		IN ExclusiveWorker& worker,
		IN uint32_t is_exit = true)
		: m_worker(worker)
		, m_is_exit(is_exit)
	{}

	inline ~ExclusiveWorkerExit()
	{
		if (m_is_exit)
		{
			m_worker.release();
		}
	}

private:
	ExclusiveWorker& m_worker;
	uint32_t m_is_exit;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif