#ifndef ECO_THREAD_CONDITION_VARIABLE_H
#define ECO_THREAD_CONDITION_VARIABLE_H
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
#include <eco/ExportApi.h>
#include <eco/thread/Mutex.h>
#ifdef ECO_WIN
#include <eco/thread/detail/ConditionVariableWin.h>
#endif


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
class ConditionVariable : public detail::ConditionVariable
{
public:
	class ScopeLock;
	inline ConditionVariable() : detail::ConditionVariable(nullptr)
	{
		set_mutex(m_mutex);
	}

private:
	eco::Mutex m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
class ConditionVariable::ScopeLock :
	public eco::Object<ConditionVariable::ScopeLock>
{
public:
	inline explicit ScopeLock(IN ConditionVariable& monitor)
		: m_monitor(m_monitor)
	{
		m_monitor.mutex().lock();
	}

	inline ~ScopeLock()
	{
		m_monitor.mutex().unlock();
	}

	inline void notify_one()
	{
		m_monitor.notify_one();
	}

	inline void notify_all()
	{
		m_monitor.notify_all();
	}

	inline void wait()
	{
		m_monitor.wait();
	}

	inline int  timed_wait(IN int milliseconds)
	{
		return m_monitor.timed_wait(milliseconds);
	}

private:
	ConditionVariable& m_monitor;
};


}// ns::eco
#endif