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
#include <eco/Object.h>
#include <mutex>
#include <vector>
#include <memory>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<typename Object>
class MemoryPool
{
public:
	inline MemoryPool()
	{
		using namespace std::placeholders;
		m_recycle = std::bind(&MemoryPool::recycle, this, _1);
	}

	inline ~MemoryPool()
	{
		clear();
	}

	inline void reserve(IN const uint32_t size)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_buffer.reserve(size);
	}

	inline std::shared_ptr<Object> create()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_buffer.size() == 0)	// add new buffer item.
		{
			m_buffer.push_back(new Object);
		}
		std::shared_ptr<Object> sp(m_buffer.back(), m_recycle);
		m_buffer.pop_back();
		return sp;
	}

	template<typename BaseObject>
	inline std::shared_ptr<BaseObject> create()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_buffer.size() == 0)	// add new buffer item.
		{
			m_buffer.push_back(new Object);
		}
		std::shared_ptr<BaseObject> sp(m_buffer.back(), m_recycle);
		m_buffer.pop_back();
		return sp;
	}

	inline void clear()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		std::for_each(m_buffer.begin(), m_buffer.end(), [](Object* heap) {
			delete heap;
		});
		m_buffer.clear();
	}

private:
	inline void recycle(IN Object* p)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_buffer.push_back(p);
	}

	std::mutex m_mutex;
	std::vector<Object*> m_buffer;
	std::function<void(Object*)> m_recycle;
};


////////////////////////////////////////////////////////////////////////////////
template<typename Object>
inline MemoryPool<Object>& memory_pool()
{
	return eco::Singleton<MemoryPool<Object>>::instance();
}
template<typename Object>
inline std::shared_ptr<Object> memory_pool_new()
{
	return memory_pool<Object>().create();
}
template<typename Object>
inline std::shared_ptr<Object> memory_pool_copy(IN const Object& obj)
{
	std::shared_ptr<Object> cpy = memory_pool_new<Object>();
	memcpy(cpy.get(), &obj, sizeof(obj));
	return cpy;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif
