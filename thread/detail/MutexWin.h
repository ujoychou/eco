#ifndef ECO_THREAD_MUTEX_WIN_H
#define ECO_THREAD_MUTEX_WIN_H
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
#include <eco/Project.h>
#include <windows.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(detail);


////////////////////////////////////////////////////////////////////////////////
class Mutex : public eco::Object<Mutex>
{
public:
	class ScopeLock;
	class OrderLock;
	class OrderRelock;
	class OrdersLock;

	inline Mutex()
	{
		// first wait spin count, if mutex not weak then get into sleep.
		const int spin_count = 5000;
		::InitializeCriticalSectionAndSpinCount(&m_mutex, spin_count);
		//::InitializeCriticalSection(&m_impl->m_mutex);
	}

	inline ~Mutex()
	{
		::DeleteCriticalSection(&m_mutex);
	}

	inline void lock()
	{
		::EnterCriticalSection(&m_mutex);
	}

	inline bool try_lock()
	{
		return ::TryEnterCriticalSection(&m_mutex) != 0;
	}

	inline void unlock()
	{
		::LeaveCriticalSection(&m_mutex);
	}

	inline CRITICAL_SECTION& raw()
	{
		return m_mutex;
	}

private:
	CRITICAL_SECTION m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
ECO_NS_END(detail);
#endif