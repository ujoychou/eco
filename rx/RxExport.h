#ifndef ECO_RX_EXPORT_H
#define ECO_RX_EXPORT_H
/*******************************************************************************
@ name
rx export.

@ function
#.rx export declear.

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.

@ ujoy modifyed on 2021-11-20
1.Export.h -> RxExport.h


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Prec.h>
#include <eco/Object.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
struct Heap {};
struct Null {};
const Heap heap;
const Null null;


////////////////////////////////////////////////////////////////////////////////
/*@ export property set. it can be reference in export header, because it's has
only one member "type_t*" which just like export object or value.
* @ iterator: property set iterator.
*/
template<class type_t>
class iterator
{
public:
	// iterator traits.
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef type_t value_type;
	typedef type_t* difference_type;
	typedef type_t* pointer;
	typedef type_t& reference;
	typedef difference_type distance_type;

public:
	explicit iterator(type_t* first_obj = 0) : m_cur(first_obj)
	{}

	inline iterator<type_t> operator++(void)
	{
		return iterator(++m_cur);
	}

	inline iterator<type_t> operator++(int)
	{
		type_t* temp = m_cur;
		++m_cur;
		return iterator(temp);
	}
	inline iterator<type_t> operator--()
	{
		return iterator(--m_cur);
	}
	inline iterator<type_t> operator--(int)
	{
		type_t* temp = m_cur;
		--m_cur;
		return iterator(temp);
	}
	inline int operator-(const iterator<type_t>& it) const
	{
		return (int)(m_cur - it.m_cur);
	}
	inline type_t* operator->()
	{
		return m_cur;
	}
	inline type_t& operator*()
	{
		return *m_cur;
	}
	inline bool operator==(const iterator<type_t>& it) const
	{
		return m_cur == it.m_cur;
	}
	inline bool operator!=(const iterator<type_t>& it) const
	{
		return m_cur != it.m_cur;
	}

private:
	type_t* m_cur;
};
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);


////////////////////////////////////////////////////////////////////////////////
#define ECO_IMPL_INIT(type_t)\
public:\
	inline void init(IN type_t&) {}

#define ECO_IMPL_API_A() \
public:\
	friend class Impl;\
	class Impl;\
	Impl& impl();\
	const Impl& impl() const;

#define ECO_IMPL_API() \
ECO_IMPL_API_A()\
protected:\
	Impl* m_impl;


// singleton: single api.
#define ECO_SINGLETON_API(object_t)\
    ECO_IMPL_API();\
    ECO_NONCOPYABLE(object_t);\
public:\
	~object_t();\
private:\
	friend class eco::Singleton<object_t>;\
	object_t();\
	

////////////////////////////////////////////////////////////////////////////////
#define ECO_TYPE_API_A(type_t) \
public:\
	type_t();\
	type_t(IN type_t&&); \
	type_t(IN eco::Null); \
	~type_t();\
	type_t& operator=(IN type_t&&);\
	type_t& operator=(IN eco::Heap);\
	type_t& operator=(IN eco::Null);\
	bool null() const;\
	void swap(IN type_t&);\
	void reserve();\
	void reset();

#define ECO_TYPE_API_1(type_t) \
ECO_TYPE_API_A(type_t) \
ECO_IMPL_API()

#define ECO_TYPE_API_2(type_t, parent_t) \
ECO_TYPE_API_A(type_t) \
ECO_IMPL_API_A()

#define ECO_TYPE_API(...) ECO_MACRO(ECO_TYPE_API_, __VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
#define ECO_VALUE_API_A(type_t) \
public:\
	type_t(IN const type_t&); \
	type_t& operator=(IN const type_t&);

#define ECO_VALUE_API_1(type_t) \
ECO_VALUE_API_A(type_t) \
ECO_TYPE_API_1(type_t)

#define ECO_VALUE_API_2(type_t, parent_t) \
ECO_VALUE_API_A(type_t) \
ECO_TYPE_API_2(type_t, parent_t)

#define ECO_VALUE_API(...) ECO_MACRO(ECO_VALUE_API_, __VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
#define ECO_OBJECT_API_A(type_t)\
public:\
	typedef std::weak_ptr<type_t> wptr;\
	typedef std::shared_ptr<type_t> ptr;\
	ECO_NONCOPYABLE(type_t)

#define ECO_OBJECT_API_1(type_t) \
ECO_OBJECT_API_A(type_t) \
ECO_TYPE_API_1(type_t)

#define ECO_OBJECT_API_2(type_t, parent_t) \
ECO_OBJECT_API_A(type_t) \
ECO_TYPE_API_2(type_t, parent_t)

#define ECO_OBJECT_API(...) ECO_MACRO(ECO_OBJECT_API_, __VA_ARGS__)

#define ECO_OBJECT_COPY_API(type_t) \
public: type_t copy() const; \
ECO_OBJECT_API(type_t)


////////////////////////////////////////////////////////////////////////////////
#define ECO_SHARED_API(type_t) \
ECO_TYPE_API_A(type_t)\
ECO_IMPL_API_A() \
public:\
	type_t(IN const type_t& obj);\
	type_t& operator=(IN const type_t& obj);\
	bool operator==(IN const type_t& obj) const;\
	bool operator!=(IN const type_t& obj) const;\
protected:\
	class Proxy; \
	Proxy* m_impl;

#define ECO_SHARED_COPY_API(type_t) \
public: type_t copy() const; \
ECO_SHARED_API(type_t)


////////////////////////////////////////////////////////////////////////////////
#endif