#ifndef ECO_THREAD_CALL_ONCE_H
#define ECO_THREAD_CALL_ONCE_H
/*******************************************************************************
@ name
thread safe call once.

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Project.h>
#include <eco/thread/State.h>
#include <eco/thread/Mutex.h>



namespace eco{;
namespace thread{;



////////////////////////////////////////////////////////////////////////////////
class CallOnce : public eco::Object<CallOnce>
{
public:
	typedef std::function<void (void)> func_t;

	inline CallOnce()
	{}

	inline void reset()
	{
		m_finish.none();
	}

	inline int finished() const
	{
		return m_finish.is_ok();
	}

	/*@ call once work funciton.*/
	inline void work(IN func_t func)
	{
		if (!m_finish.is_ok())	// double check.
		{
			eco::Mutex::ScopeLock lock(m_finish_mutex);
			if (!m_finish.is_ok())
			{
				func();
				m_finish.ok();
			}
		}// end if.
	}

private:
	eco::atomic::State m_finish;
	eco::Mutex m_finish_mutex;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif