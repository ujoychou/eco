#ifndef ECO_THREAD_MUTEX_H
#define ECO_THREAD_MUTEX_H
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
#ifdef ECO_WIN
#include <eco/thread/detail/MutexWin.h>
#endif

namespace eco{;

typedef detail::Mutex Mutex;
////////////////////////////////////////////////////////////////////////////////
class Mutex::ScopeLock : public eco::Object<Mutex::ScopeLock>
{
public:
	inline explicit ScopeLock(IN Mutex& mutex) : m_mutex(&mutex)
	{
		m_mutex->lock();
	}
	inline explicit ScopeLock(IN Mutex& mutex, IN bool is_lock)
		: m_mutex(is_lock ? &mutex : nullptr)
	{
		if (m_mutex != nullptr)
			m_mutex->lock();
	}
	inline ~ScopeLock()
	{
		if (m_mutex != nullptr)
			m_mutex->unlock();
	}

private:
	Mutex* m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
class Mutex::OrderLock : public eco::Object<Mutex::OrderLock>
{
public:
	// lock the two mutex in order.
	inline OrderLock(IN Mutex& mutex1, IN Mutex& mutex2)
		: m_mutex1(mutex1), m_mutex2(mutex2)
	{
		if (&m_mutex1 == &mutex2)
		{
			m_mutex1.lock();
		}
		else if (&m_mutex1 < &m_mutex2)
		{
			m_mutex1.lock();
			m_mutex2.lock();
		}
		else
		{
			m_mutex2.lock();
			m_mutex1.lock();
		}
	}

	inline ~OrderLock()
	{
		if (&m_mutex1 == &m_mutex2)
		{
			m_mutex1.unlock();
		}
		else
		{
			m_mutex1.unlock();
			m_mutex2.unlock();
		}
	}

private:
	Mutex& m_mutex1;
	Mutex& m_mutex2;
};


////////////////////////////////////////////////////////////////////////////////
class Mutex::OrderRelock : public eco::Object<Mutex::OrderRelock>
{
public:
	// mutex1 is already locked by scope_lock, mutex2 is unknown, so we will
	// try to lock it. last lock the two mutex in order.
	inline OrderRelock(IN Mutex& mutex1, IN Mutex& mutex2)
		: m_unlock(true), m_mutex2(mutex2)
	{
		if (&mutex1 == &mutex2)
		{
			m_unlock = false;
		}
		// if two mutex is in order.
		else if (&mutex1 < &mutex2)
		{
			mutex2.lock();
			return;
		}
		// try to lock it not in order to avoid dead lock.
		else if (!mutex2.try_lock()) {
			// relock the mutex in order.
			mutex1.unlock();
			mutex2.lock();
			mutex1.lock();
		}
	}

	inline ~OrderRelock()
	{
		if (m_unlock)
		{
			m_mutex2.unlock();
		}
	}

private:
	uint32_t m_unlock;
	Mutex& m_mutex2;
};


////////////////////////////////////////////////////////////////////////////////
class Mutex::OrdersLock : public eco::Object<Mutex::OrdersLock>
{
public:
	
	inline OrdersLock()
	{}

	inline ~OrdersLock()
	{
		unlock();
	}

	// lock the mutexs in order.
	inline void push_back(IN Mutex& mutex)
	{
		auto it = m_mutexs.begin();
		for (; it != m_mutexs.end(); ++it)
		{
			if (&mutex < *it)
				break;
		}
		m_mutexs.insert(it, &mutex);
	}

	// lock the mutexs
	inline void lock()
	{
		for (auto it = m_mutexs.begin(); it != m_mutexs.end(); ++it)
		{
			(**it).lock();
		}
	}

	// unlock the mutexs
	inline void unlock()
	{
		for (auto it = m_mutexs.begin(); it != m_mutexs.end(); ++it)
		{
			(**it).unlock();
		}
	}

private:
	std::list<Mutex*> m_mutexs;
};


////////////////////////////////////////////////////////////////////////////////
template<typename Aspect, uint32_t mutex_size>
class MutexPool
{
public:
	inline static Mutex& mutex(const void* obj)
	{
		return s_mutex_pool[reinterpret_cast<uint32_t>(obj) % mutex_size];
	}

private:
	static Mutex s_mutex_pool[mutex_size];
};
template<typename Aspect, uint32_t mutex_size>
Mutex MutexPool<Aspect, mutex_size>::s_mutex_pool[mutex_size];


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif