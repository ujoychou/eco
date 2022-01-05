#ifndef ECO_SAFETY_H
#define ECO_SAFETY_H
/*******************************************************************************
@ name


@ function
#1.lock_ptr: other can't read and write.
#2.shared_lock_ptr: other can read but can't write.
#3.reader_lock_ptr=shared_lock_ptr.
#4.unique_lock_ptr: other can't read and write.
#5.writer_lock_ptr=unique_lock_ptr.

#6.erased_lock_ptr: after erased setting, write operation can't be use.
#7.erased_lock_ptr.Safety: using lock_ptr.
#8.erased_lock_ptr.SharedSafety: using shared_lock_ptr.(no unique_lock_ptr)


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy created on 2020-08-24.
1.init this class.


--------------------------------------------------------------------------------
* copyright(c) 2020 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Object.h>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<class object_t, class safety_t>
class BaseSafetyPtr
{
	ECO_NONCOPYABLE(BaseSafetyPtr);
public:
	typedef safety_t safety_t;
	typedef BaseSafetyPtr<object_t, safety_t> BasePtr;
	inline BaseSafetyPtr() : m_ptr(0), m_safety(0)
	{}

	inline BaseSafetyPtr(const safety_t& safety) : m_ptr(0), m_safety(0)
	{
		set_safety(safety);
	}

	inline BaseSafetyPtr(BaseSafetyPtr&& obj)
	{
		m_ptr = obj.m_ptr;
		m_safety = obj.m_safety;
		obj.clear();
	}

	inline ~BaseSafetyPtr()
	{
		unlock();
	}

	inline operator bool() const
	{
		return m_ptr != nullptr;
	}

	inline void unlock()
	{
		if (m_safety != nullptr)
		{
			m_safety->mutex().unlock();
			m_safety = nullptr;
			m_ptr = nullptr;
		}
	}

	inline void clear()
	{
		m_ptr = nullptr;
		m_safety = nullptr;
	}

	inline void reset(const safety_t& safety)
	{
		unlock();
		set_safety(safety);
	}

protected:
	inline void set_safety(const safety_t& safety)
	{
		if (!safety) return;
		m_safety = (safety_t*)&safety;
		m_safety->mutex().lock();
		if (*m_safety)
		{
			m_ptr = (object_t*)m_safety;
			return;
		}
		m_safety = nullptr;
	}

	object_t* m_ptr;
	safety_t* m_safety;
};


template<class object_t> class Safety;
template<class object_t> class SharedSafety;
////////////////////////////////////////////////////////////////////////////////
template<class object_t>
class SafetyPtr : public BaseSafetyPtr<object_t, Safety<object_t>>
{
	ECO_PTR_MEMBER(object_t, m_ptr);
	inline SafetyPtr() : BasePtr() {}
	inline SafetyPtr(const Safety<object_t>& safe) : BasePtr(safe) {}
	inline SafetyPtr(SafetyPtr&& obj) : BasePtr(std::move(obj)) {}
};
template<class object_t>
class WriterPtr : public BaseSafetyPtr<object_t, SharedSafety<object_t>>
{
	ECO_PTR_MEMBER(object_t, m_ptr);
	inline WriterPtr() : BasePtr() {}
	inline WriterPtr(const SharedSafety<object_t>& safe) : BasePtr(safe) {}
	inline WriterPtr(WriterPtr&& obj) : BasePtr(std::move(obj)) {}
};
template<class object_t>
class ReaderPtr : public BaseSafetyPtr<object_t, SharedSafety<object_t>>
{
public:
	inline ReaderPtr() : BasePtr() {}
	inline ReaderPtr(ReaderPtr&& obj) : BasePtr(std::move(obj)) {}
	inline ReaderPtr(const safety_t& safe) : BasePtr()
	{
		set_safety(safe);
	}

	inline ~ReaderPtr()
	{
		unlock();
	}

	inline void unlock()
	{
		if (m_safety == nullptr) return;
		m_safety->mutex().unlock_shared();
		m_safety = nullptr;
		m_ptr = nullptr;
	}

	inline void reset(const safety_t& safety)
	{
		unlock();
		set_safety(safety);
	}

public:
	inline const object_t* operator->() const
	{
		return m_ptr;
	}
	inline const object_t& operator*() const
	{
		return *m_ptr;
	}
	inline const object_t* get() const
	{
		return m_ptr;
	}
	inline bool null() const
	{
		return m_ptr == nullptr;
	}

protected:
	inline void set_safety(const safety_t& safety)
	{
		if (!safety) return;
		m_safety = (safety_t*)&safety;
		m_safety->mutex().lock_shared();
		m_ptr = (object_t*)m_safety;
	}
};


//##############################################################################
//##############################################################################
template<class object_t>
class Safety : public eco::Object<object_t>
{
private:
	friend class BaseSafetyPtr<object_t, Safety<object_t>>;

	// object thread safe lock.
	mutable std::mutex m_mutex;

	// object in a erasing state: writer can't be get privilege.
	mutable std::atomic<uint32_t> m_erased;

public:
	typedef Safety<object_t> safety;

	inline Safety()
	{
		m_erased = 0;
	}

	inline operator bool() const
	{
		return m_erased == 0;
	}

	inline Safety& erased()
	{
		m_erased = 1;
		return *this;
	}

	inline SafetyPtr<object_t> lock_ptr()
	{
		return (*this) ? SafetyPtr<object_t>() : SafetyPtr<object_t>(*this);
	}

	inline SafetyPtr<object_t> erased_lock_ptr()
	{
		erased(v);
		return SafetyPtr<object_t>(*this);
	}
};


////////////////////////////////////////////////////////////////////////////////
template<class object_t>
class SharedSafety : public eco::Object<object_t>
{
private:
	friend class WriterPtr<object_t>;
	friend class ReaderPtr<object_t>;

	// object thread safe lock.
	mutable boost::shared_mutex m_mutex;

	// object in a erasing state: writer can't be get privilege.
	mutable std::atomic<uint32_t> m_erased;

public:
	typedef SharedSafety<object_t> safety;

	inline SharedSafety()
	{
		m_erased = 0;
	}

	inline operator bool() const
	{
		return m_erased == 0;
	}

	inline void set_erased()
	{
		m_erased = 1;
	}

	// writer privilege.
	inline WriterPtr<object_t> unique_lock_ptr()
	{
		return (*this) ? WriterPtr<object_t>() : WriterPtr<object_t>(*this);
	}
	inline WriterPtr<object_t> writer_lock_ptr()
	{
		return unique_lock_ptr();
	}
	
	// reader privilege.
	inline ReaderPtr<object_t> shared_lock_ptr() const
	{
		return (*this) ? ReaderPtr<object_t>() : ReaderPtr<object_t>(*this);
	}
	inline ReaderPtr<object_t> reader_lock_ptr() const
	{
		return shared_lock_ptr();
	}
	
	// erasing privilege.
	inline ReaderPtr<object_t> erased_lock_ptr()
	{
		set_erased();
		return ReaderPtr<object_t>(*this);
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif