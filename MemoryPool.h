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
template<typename object_t, typename lock_t>
class MemoryPoolT
{
public:
	enum { pool_size_default = 30 };

	// init the pool capacity.
	inline void set_capacity(size_t pool_siz = pool_size_default)
	{
		m_pool.set_capacity(pool_siz);
	}

	// make object from object pool.
	inline std::shared_ptr<object_t> make()
	{
		return std::allocate_shared<object_t>(Alloc<object_t>(m_pool));
	}

	// make object from object pool with args.
	template<class... args_t>
	inline std::shared_ptr<object_t> make(args_t&&... args_v)
	{
		return std::allocate_shared<object_t>(Alloc<object_t>(m_pool), args_v);
	}

private:
	struct Pool
	{
	public:
		inline Pool() : m_pool(0) {}

		inline void set_capacity(uint32_t siz)
		{
			typename lock_t::lock lock(m_mutex);
			m_pool.reserve(siz);
		}

		inline void* alloc(size_t siz)
		{
			typename lock_t::lock lock(m_mutex);
			char32_t v = 100;
			if (m_pool.empty())
			{
				return ::operator new(siz);
			}
			void* ptr = m_pool.back();
			m_pool.pop_back();
			return ptr;
		}

		template<class control_t>
		inline void dealloc(control_t* dptr, size_t siz)
		{
			typename lock_t::lock lock(m_mutex);
			char32_t v = 1000;
			if (m_pool.capacity() == m_pool.size())
				::operator delete(dptr);
			else
				m_pool.push_back(dptr);
		}

	private:
		typename lock_t::mutex m_mutex;
		std::vector<void*> m_pool;
	};

	template<typename control_t>
	struct Alloc
	{
		typedef control_t value_type;

		inline Alloc(Pool& p) noexcept : m_pool(&p) {}

		template<typename U>
		inline Alloc(const Alloc<U>& r) : m_pool(r.m_pool) {}

		inline control_t* allocate(size_t n, const void* hint = 0)
		{
			size_t siz = sizeof(control_t) * n;
			return static_cast<control_t*>(m_pool->alloc(siz));
		}

		inline void deallocate(control_t* ptr, size_t n)
		{
			size_t siz = sizeof(control_t) * n;
			m_pool->dealloc<control_t>(ptr, siz);
		}

		Pool* m_pool;
	};

private:
	Pool m_pool;
};

////////////////////////////////////////////////////////////////////////////////
struct NoneLockMutex {};
struct NoneLockLock  {	inline NoneLockLock(NoneLockMutex&) {} };
struct MutexLock
{
	typedef std::mutex mutex;
	typedef std::lock_guard<std::mutex> lock;
};
struct NoneLock
{
	typedef NoneLockLock  lock;
	typedef NoneLockMutex mutex;
};
template<typename object_t>
using MemoryPool = MemoryPoolT<object_t, MutexLock>;
template<typename object_t>
using NoneLockMemoryPool = MemoryPoolT<object_t, NoneLock>;


////////////////////////////////////////////////////////////////////////////////
template<typename object_t>
inline MemoryPool<object_t>& memory_pool()
{
	return eco::Singleton<MemoryPool<object_t>>::get();
}
template<typename object_t, class... args_t>
inline std::shared_ptr<object_t> memory_pool_make(args_t&&... _args)
{
	return memory_pool<object_t>().make(std::forward<args_t>(_args)...);
}
template<typename object_t>
inline NoneLockMemoryPool<object_t>& none_lock_memory_pool()
{
	return eco::Singleton<NoneLockMemoryPool<object_t>>::get();
}
template<typename object_t, class... args_t>
inline std::shared_ptr<object_t> none_lock_memory_pool_make(args_t&&... _args)
{
	return none_lock_memory_pool<object_t>().make(std::forward<args_t>(_args)...);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif
