#ifndef ECO_THREAD_CONDVAR_WIN_H
#define ECO_THREAD_CONDVAR_WIN_H
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
#include <eco/rx/RxExport.h>
#include <eco/std/mutex.h>


ECO_NS_BEGIN(eco);
namespace detail{;


////////////////////////////////////////////////////////////////////////////////
class ConditionVariable : public eco::Object<ConditionVariable>
{
public:
	inline explicit ConditionVariable(
		IN std_mutex* m = nullptr) : m_mutex(m)
	{
		// condition var no need to destroy.
		::InitializeConditionVariable(&m_cond_var);
	}
	inline void set_mutex(IN std_mutex& m)
	{
		m_mutex = &m;
	}
	inline std_mutex& mutex()
	{
		return *m_mutex;
	}

	inline void wait()
	{
		::SleepConditionVariableCS(&m_cond_var, &m_mutex->raw(), INFINITE);
	}
	inline int timed_wait(IN uint32_t milliseconds)
	{
		return ::SleepConditionVariableCS(
			&m_cond_var, &m_mutex->raw(), milliseconds);
	}
	inline void notify_one()
	{
		::WakeConditionVariable(&m_cond_var);
	}
	inline void notify_all()
	{
		::WakeAllConditionVariable(&m_cond_var);
	}

private:
	std_mutex* m_mutex;
	CONDITION_VARIABLE m_cond_var;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif