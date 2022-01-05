#ifndef ECO_MEMORY_H
#define ECO_MEMORY_H
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
#include <eco/Def.h>
#include <mutex>
#include <vector>
#include <algorithm>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
#define ECO_PTR_MEMBER(data_t, m_data) \
public:\
	inline data_t* operator->()\
	{\
		return m_data;\
	}\
	inline const data_t* operator->() const\
	{\
		return m_data;\
	}\
	inline data_t& operator*()\
	{\
		return *m_data;\
	}\
	inline const data_t& operator*() const\
	{\
		return *m_data;\
	}\
	inline data_t* get()\
	{\
		return m_data;\
	}\
	inline const data_t* get() const\
	{\
		return m_data;\
	}\
	inline bool null() const\
	{\
		return m_data == nullptr;\
	}

////////////////////////////////////////////////////////////////////////////////
/* auto delete heap ptr and manage object memory.*/
template<typename Object>
class AutoDelete
{
	ECO_PTR_MEMBER(Object, m_ptr);
public:
	inline AutoDelete(
		IN Object* ptr = nullptr,
		IN bool auto_delete = true)
		: m_ptr(ptr), m_auto_delete(auto_delete)
	{}

	inline ~AutoDelete()
	{
		if (m_auto_delete)
		{
			delete m_ptr;
		}
	}

	inline void reset(Object* ptr = nullptr, bool auto_delete = true)
	{
		if (m_ptr == ptr)
		{
			return;
		}

		if (m_auto_delete)
		{
			delete m_ptr;
		}

		m_ptr = ptr;
		m_auto_delete = auto_delete;
	}

	inline void release()
	{
		m_ptr = nullptr;
		m_auto_delete = false;
	}

private:
	Object* m_ptr;
	bool m_auto_delete;
};


////////////////////////////////////////////////////////////////////////////////
template<typename T>
class AutoArray
{
	ECO_NONCOPYABLE(AutoArray);
public:
	typedef std::shared_ptr<AutoArray> ptr;

	inline AutoArray()
	{}

	inline ~AutoArray()
	{
		for (size_t i = 0; i < m_data.size(); i++)
		{
			delete ((T*)m_data[i]);
		}
	}

	inline T* release(IN size_t i)
	{
		T* data = (T*)m_data[i];
		m_data[i] = nullptr;
		return data;
	}

	inline void release()
	{
		m_data.clear();
	}

	inline size_t size() const
	{
		return m_data.size();
	}

	inline T* operator [](const size_t i)
	{
		return (T*)m_data[i];
	}

	inline const T* operator [](const size_t i) const
	{
		return (T*)m_data[i];
	}

	inline std::vector<void*>& get()
	{
		return m_data;
	}

	inline const std::vector<void*>& get() const
	{
		return m_data;
	}

private:
	std::vector<void*> m_data;
};


////////////////////////////////////////////////////////////////////////////////
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
		return std::allocate_shared<object_t>(
			Alloc<object_t>(m_pool), std::forward<args_t>(args_v)...);
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
			m_pool->dealloc(ptr, sizeof(control_t) * n);
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
template<typename object_t, typename container>
inline void release(IN container& c)
{
	std::for_each(c.begin(), c.end(), [&](IN object_t* pitem) {
		delete pitem;
	});
	c.clear();
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif
