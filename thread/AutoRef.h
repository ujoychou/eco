#ifndef ECO_AUTO_REF_H
#define ECO_AUTO_REF_H
/*******************************************************************************
@ name

@ function

@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/thread/Atomic.h>



namespace eco{;


////////////////////////////////////////////////////////////////////////////////
// auto ref count to manage object life cycle.
#define ECO_OBJECT_AUTOREF()\
public:\
	inline void add_ref()\
	{\
		++m_ref;\
	}\
	inline void del_ref()\
	{\
		if (--m_ref == 0)\
			delete this;\
	}\
	inline uint32_t ref_size() const\
	{\
		return m_ref;\
	}\
private:\
	eco::Atomic<uint32_t> m_ref;


////////////////////////////////////////////////////////////////////////////////
// auto ref ptr help manage auto ref object life cycle.
template<typename ObjectT>
class AutoRefPtr
{
public:
	inline ~AutoRefPtr()
	{
		reset(nullptr);
	}

	// add ref operation.
	inline explicit AutoRefPtr(ObjectT* obj_ptr = nullptr) : m_obj_ptr(obj_ptr)
	{
		add_ref();
	}

	inline AutoRefPtr(IN const AutoRefPtr& other) : m_obj_ptr(other.m_obj_ptr)
	{
		add_ref();
	}

	inline AutoRefPtr& operator=(IN const AutoRefPtr& other)
	{
		reset(other.m_obj_ptr);
		return *this;
	}

	inline AutoRefPtr(AutoRefPtr&& other) : m_obj_ptr(other.m_obj_ptr)
	{
		other.m_obj_ptr = nullptr;
	}
	inline AutoRefPtr& operator=(IN AutoRefPtr&& other)
	{
		move(other);
		return *this;
	}

	inline void reset(ObjectT* obj_ptr)
	{
		if (m_obj_ptr != nullptr)		// del old object.
			m_obj_ptr->del_ref();
		m_obj_ptr = obj_ptr;			// set new object.
		add_ref();
	}

public:
	inline ObjectT* operator->()
	{
		return m_obj_ptr;
	}
	inline const ObjectT* operator->() const
	{
		return m_obj_ptr;
	}
	inline ObjectT& operator*()
	{
		return *m_obj_ptr;
	}
	inline const ObjectT& operator*() const
	{
		return *m_obj_ptr;
	}

	inline void swap(IN AutoRefPtr& other)
	{
		std::swap(m_obj_ptr, other.m_obj_ptr);
	}

	inline void move(IN AutoRefPtr& other)
	{
		if (m_obj_ptr != nullptr)
			m_obj_ptr->del_ref();
		m_obj_ptr = other.m_obj_ptr;
		other.m_obj_ptr = nullptr;
	}

	inline bool null() const
	{
		return m_obj_ptr == nullptr;
	}

private:
	inline void add_ref()
	{
		if (m_obj_ptr != nullptr)
		{
			m_obj_ptr->add_ref();
		}
	}

	ObjectT* m_obj_ptr;
};


////////////////////////////////////////////////////////////////////////////////
}
#endif