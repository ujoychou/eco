#ifndef ECO_THREAD_EXCLUSIVE_DATA_H
#define ECO_THREAD_EXCLUSIVE_DATA_H
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


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
class ExclusiveData
{
public:
	inline ExclusiveData()
	{}

	inline eco::Mutex& mutex()
	{
		return m_mutex;
	}

	inline bool busy() const
	{
		return m_state.is_ok();
	}

	inline bool occupy(IN const int state)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		if (m_state != state)
		{
			return false;
		}
		m_state.ok();
		return true;
	}

	inline void set_state(IN const int state)
	{
		m_state = state;
	}

	inline const int get_state() const
	{
		return m_state;
	}

protected:
	eco::Mutex m_mutex;
	eco::atomic::State m_state;
};


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif