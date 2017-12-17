#ifndef ECO_MEMORY_POOL_H
#define ECO_MEMORY_POOL_H
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-12-03.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/thread/Mutex.h>
#include <eco/Object.h>
#include <vector>
#include <memory>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<typename Object>
class MemoryPool
{
public:
	inline void reserve(IN const uint32_t size)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		m_buffer.reserve(size);
	}

	inline std::shared_ptr<Object> create()
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		if (m_buffer.size() == 0)	// add new buffer item.
		{
			m_buffer.push_back(new Object);
		}

		std::shared_ptr<Object> sp(m_buffer.back(),
			std::bind(&MemoryPool::recycle, this, std::placeholders::_1));
		m_buffer.pop_back();
		return sp;
	}

	template<typename BaseObject>
	inline std::shared_ptr<BaseObject> create()
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		if (m_buffer.size() == 0)	// add new buffer item.
		{
			m_buffer.push_back(new Object);
		}

		std::shared_ptr<BaseObject> sp(m_buffer.back(),
			std::bind(&MemoryPool::recycle, this, std::placeholders::_1));
		m_buffer.pop_back();
		return sp;
	}

	inline void clear()
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		std::for_each(m_buffer.begin(), m_buffer.end(),
			[](Object* heap_ptr) {
			delete heap_ptr;
		});
		m_buffer.clear();
	}

	inline ~MemoryPool()
	{
		clear();
	}

private:
	inline void recycle(IN Object* p)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		m_buffer.push_back(p);
	}

	eco::Mutex m_mutex;
	std::vector<Object*> m_buffer;
};


////////////////////////////////////////////////////////////////////////////////
template<typename Object>
inline MemoryPool<Object>& get_memory_pool()
{
	return eco::Singleton<MemoryPool<Object>>::instance();
}

#define ECO_POOL_NEW(object_t) get_memory_pool<object_t>().create()
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif
