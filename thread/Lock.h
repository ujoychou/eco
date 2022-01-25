#ifndef ECO_THREAD_LOCK_H
#define ECO_THREAD_LOCK_H
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
#include <eco/Memory.h>
#include <eco/std/mutex.h>
#include <list>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class OrderLock : public eco::Object<Mutex::OrderLock>
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
class OrderRelock : public eco::Object<Mutex::OrderRelock>
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
class OrdersLock : public eco::Object<Mutex::OrdersLock>
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
template<typename aspect_t, uint32_t mutex_size>
class MutexPool
{
public:
	inline static std_mutex& mutex(const uint64_t id)
	{
		return s_mutex_pool[(uint32_t)(id) % mutex_size];
	}

	inline static std_mutex& mutex(const void* obj)
	{
		return s_mutex_pool[reinterpret_cast<size_t>(obj) % mutex_size];
	}

private:
	static std_mutex s_mutex_pool[mutex_size];
};
template<typename aspect_t, uint32_t mutex_size>
std_mutex MutexPool<aspect_t, mutex_size>::s_mutex_pool[mutex_size];


////////////////////////////////////////////////////////////////////////////////
template<class object_t>
class LockPtr
{
	ECO_NONCOPYABLE(LockPtr);
	ECO_PTR_MEMBER(object_t, m_ptr);
public:
	inline LockPtr() : m_ptr(0), m_mutex(0)
	{}

	inline LockPtr(std_mutex& m, const object_t& obj)
		: m_ptr(const_cast<object_t*>(&obj)), m_mutex(&m)
	{
		m_mutex->lock();
	}

	inline LockPtr(LockPtr&& obj)
	{
		m_ptr = obj.m_ptr;
		m_mutex = obj.m_mutex;
		obj.clear();
	}

	inline ~LockPtr()
	{
		unlock();
	}

	inline operator bool() const
	{
		return m_ptr != nullptr;
	}

	inline void unlock()
	{
		if (m_mutex != nullptr)
		{
			m_mutex->unlock();
			m_mutex = nullptr;
			m_ptr = nullptr;
		}
	}

	inline void clear()
	{
		m_ptr = nullptr;
		m_mutex = nullptr;
	}

	inline void reset(std_mutex& m)
	{
		unlock();
		m_ptr = const_cast<object_t*>(&obj);
		m_mutex = &m;
		m_mutex->lock();
	}

private:
	object_t* m_ptr;
	std_mutex* m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
template<class object_t>
inline LockPtr<object_t> lock_ptr(const object_t& obj)
{
	return LockPtr<object_t>(eco::MutexPool<int, 50>::mutex(&obj), obj);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif