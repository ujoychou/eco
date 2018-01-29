#ifndef ECO_EXPORT_H
#define ECO_EXPORT_H
/*******************************************************************************
@ name
dll export template declare.

@ function
1.note macro for coding.
2.object api declare.
3.value api declare.
4.

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <cstdint>
#include <xutility>
#include <memory>


////////////////////////////////////////////////////////////////////////////////
/*@ eco namespace.*/
namespace eco{}
#define ECO_NS_BEGIN(ns) namespace ns{
#define ECO_NS_END(ns) }


/*@ c++ 11.*/
#ifdef ECO_NO_CXX11
#	define nullptr (NULL)
#	define override
#endif

#ifdef ECO_VC100
#define ECO_NO_VARIADIC_TEMPLATE
#endif

/*@ include protobuf.*/
#ifdef ECO_NO_PROTOBUF
#endif


////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#	define ECO_WIN32
#	define ECO_WIN
#endif

/*@ [WIN_32]*/
#ifdef ECO_WIN32
// windows related header file.
#	include <SDKDDKVer.h>
#endif


////////////////////////////////////////////////////////////////////////////////
/*@ [LINUX]*/
#ifdef ECO_LINUX
#endif


////////////////////////////////////////////////////////////////////////////////
/*@ note macro for coding.
* @ macro.IN: mark function input parameter.
* @ macro.OUT: mark function output parameter, When the function is defined, the
output parameters are located in front of the input parameters.
*/
#ifndef IN
#	define IN
#endif

#ifndef OUT
#	define OUT
#endif

#ifndef TODO
#	define TODO(something)
#endif

#ifndef NOTE
#	define NOTE(something)
#endif


#ifdef ECO_LINUX
#	define FCALL
#endif

#ifndef FCALL
#	define FCALL __cdecl
#endif

ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<typename type_t = int>
class TypeCount
{
public:
	static uint32_t s_type_count;
};
template<typename type_t>
uint32_t TypeCount<type_t>::s_type_count = 0;

template<typename type_t>
class TypeId
{
public:
	inline operator const uint32_t() const
	{
		return value;
	}
	static uint32_t value;
};
template<typename type_t>
uint32_t TypeId<type_t>::value = TypeCount<>::s_type_count++;

template<typename type_t>
inline const char* get_type()
{
	static const char* s_v = typeid(type_t).name();
	return s_v;
}
ECO_NS_END(eco);


////////////////////////////////////////////////////////////////////////////////
#define ECO_TYPE(object_t)\
public:\
	inline static const uint32_t type_id()\
	{\
		return eco::TypeId<object_t>::value;\
	}\
	virtual const uint32_t get_type_id() const\
	{\
		return type_id();\
	}\
	inline static const char* type()\
	{\
		return #object_t;\
	}\
	virtual const char* get_type() const\
	{\
		return type();\
	}

////////////////////////////////////////////////////////////////////////////////
#define ECO_NONCOPYABLE(object_t) \
private:\
	object_t(IN const object_t& );\
	object_t& operator=(IN const object_t& );

#define ECO_IMPL_API() \
public:\
	friend class Impl;\
	class Impl;\
	inline Impl& impl();\
	inline const Impl& impl() const;\
protected:\
	Impl* m_impl;

#define ECO_TYPE_API(type_t) \
public:\
	~type_t();\
	type_t();\
	ECO_IMPL_API()

#define ECO_VALUE_API(value_t) \
public:\
	value_t(IN const value_t&); \
	value_t& operator=(IN const value_t&);\
	ECO_TYPE_API(value_t)

#define ECO_OBJECT_API(object_t)\
public:\
	typedef std::weak_ptr<object_t> wptr;\
	typedef std::shared_ptr<object_t> ptr;\
	object_t(IN eco::Null);\
	object_t& operator=(IN eco::Heap);\
	object_t& operator=(IN eco::Null);\
	bool null() const;\
	void reset();\
	ECO_TYPE_API(object_t)\
	ECO_NONCOPYABLE(object_t)

#define ECO_MOVABLE_API(object_t)\
public:\
	object_t(IN object_t&& moved);\
	object_t& operator=(IN object_t&& moved);\
	ECO_OBJECT_API(object_t)


////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(eco);
// for create shared object.
struct Heap {};
struct Null {};
const Heap heap;
const Null null;
ECO_NS_END(eco);

#define ECO_SHARED_API(object_t) \
public:\
	object_t();\
	~object_t();\
	object_t(IN eco::Null);\
	object_t(IN object_t&& obj);\
	object_t(IN const object_t& obj);\
	object_t& operator=(IN object_t&& obj);\
	object_t& operator=(IN const object_t& obj);\
	object_t& operator=(IN eco::Heap);\
	object_t& operator=(IN eco::Null);\
	bool operator==(IN const object_t& obj) const;\
	bool operator!=(IN const object_t& obj) const;\
	bool null() const;\
	void swap(IN object_t& obj);\
	void reset();\
	void reserve();\
public:\
	class Proxy;\
	class Impl;\
	inline Impl& impl() const;\
protected:\
	Proxy* m_proxy;

#define ECO_IMPL_INIT(type_t)\
public:\
	inline void init(IN type_t&) {}
////////////////////////////////////////////////////////////////////////////////


ECO_NS_BEGIN(eco);

////////////////////////////////////////////////////////////////////////////////
// empty vlaue, using for init.
enum { value_none = 0 };


////////////////////////////////////////////////////////////////////////////////
template<typename ObjectT>
inline void add(ObjectT& obj, uint32_t v)
{
	obj |= v;
}
template<typename ObjectT>
inline void del(ObjectT& obj, uint32_t v)
{
	obj &= ~v;
}
template<typename ObjectT>
inline void toggle(ObjectT& obj, uint32_t v)
{
	obj ^= v;
}
template<typename ObjectT>
inline void set(ObjectT& obj, uint32_t v, bool is)
{
	obj = is ? (obj | v) : (obj & ~v);
}
template<typename ObjectT>
inline void set_v(ObjectT& obj, const uint32_t add_v, const uint32_t del_v)
{
	eco::add(add_v);
	eco::del(del_v);
}
template<typename ObjectT>
inline bool has(const ObjectT& obj, uint32_t v)
{
	return (obj & v) > 0;
}


////////////////////////////////////////////////////////////////////////////////
/*@ export property set. it can be reference in export header, because it's has
only one member "object_t*" which just like export object or value.
* @ iterator: property set iterator.
*/
template<class object_t>
class iterator
{
public:
	// iterator traits.
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef object_t value_type;
	typedef object_t* difference_type;
	typedef object_t* pointer;
	typedef object_t& reference;
	typedef difference_type distance_type;

public:
	explicit iterator(object_t* first_obj = 0) : m_cur(first_obj)
	{}

	inline iterator<object_t> operator++(void)
	{
		return iterator(++m_cur);
	}

	inline iterator<object_t> operator++(int)
	{
		object_t* temp = m_cur;
		++m_cur;
		return iterator(temp);
	}
	inline iterator<object_t> operator--()
	{
		return iterator(--m_cur);
	}
	inline iterator<object_t> operator--(int)
	{
		object_t* temp = m_cur;
		--m_cur;
		return iterator(temp);
	}
	inline int operator-(const iterator<object_t>& it) const
	{
		return (int)(m_cur - it.m_cur);
	}
	inline object_t* operator->()
	{
		return m_cur;
	}
	inline object_t& operator*()
	{
		return *m_cur;
	}
	inline bool operator==(const iterator<object_t>& it) const
	{
		return m_cur == it.m_cur;
	}
	inline bool operator!=(const iterator<object_t>& it) const
	{
		return m_cur != it.m_cur;
	}

private:
	object_t* m_cur;
};


////////////////////////////////////////////////////////////////////////////////
// object id 1: global object.
typedef uint64_t ObjectId;

// object id 2: attached object.
class AspectId
{
public:
	uint64_t object_id;
	uint32_t aspect_id;
};


ECO_NS_END(eco);
#endif