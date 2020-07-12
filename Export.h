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
#include <string>
#include <functional>
#include <assert.h>


#undef min
#undef max
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
#define ECO_NO_FUNCTION_TEMPLATE_DEFAULT
#define __func__ eco::func(__FUNCTION__)
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


////////////////////////////////////////////////////////////////////////////////
/* macro overload method. and eco using method 1.
1.using "GET_MACRO".
#define MACRO_1(a)
#define MACRO_2(a, b)
#define GET_MACRO(_1, _2, NAME, ...) NAME
#define MACRO(...) GET_MACRO(__VA_ARGS__, MACRO_2, MACRO_1)(__VA_ARGS__)
exp: ECO_BTASK

2.using "PP_CAT & PP_GETN"
#define MACRO_1(a)
#define MACRO_2(a, b)
#define MACRO_3(a, b, c)
#define MACRO(...) PP_CAT(MACRO_, PP_GETN(__VA_ARGS__))(__VA_ARGS__)
boost impl: <boost/preprocessor/variadic/size.hpp>
BOOST_PP_CAT & BOOST_PP_VARIADIC_SIZE;
*/
#define ECO_MACRO_CAT_(a, b) a##b
#define ECO_MACRO_CAT(a, b) ECO_MACRO_CAT_(a, b)
#define ECO_MACRO_STR_(a) #a
#define ECO_MACRO_STR(a) ECO_MACRO_STR_(a)
#define ECO_MACRO_GETN_(n0,n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,\
n16,n17,n18,n19,n20,n21,n22,n23,n24,n25,n26,n27,n28,n29,n30,n31,size,...) size
// ECO_MACRO_CAT_WIN for windows and linux.
#ifdef ECO_WIN
#	define ECO_MACRO_CAT_WIN ECO_MACRO_CAT
#else
#	define ECO_MACRO_CAT_WIN_(a, b) a
#	define ECO_MACRO_CAT_WIN(a, b) ECO_MACRO_CAT_WIN_(a, b)
#endif
#define ECO_MACRO_GETN(...) ECO_MACRO_CAT_WIN(ECO_MACRO_GETN_(__VA_ARGS__,32,\
31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,\
3,2,1,),)
#define ECO_MACRO(macro, ...) ECO_MACRO_CAT(ECO_MACRO_CAT(macro,\
ECO_MACRO_GETN(__VA_ARGS__))(__VA_ARGS__), )

////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(eco);
struct Heap {};
struct Null {};
const Heap heap;
const Null null;
ECO_NS_END(eco);


////////////////////////////////////////////////////////////////////////////////
#define ECO_IMPL_INIT(type_t)\
public:\
	inline void init(IN type_t&) {}

#define ECO_NONCOPYABLE(type_t) \
private:\
	type_t(IN const type_t& );\
	type_t& operator=(IN const type_t& );

#define ECO_IMPL_API_A() \
public:\
	friend class Impl;\
	class Impl;\
	inline Impl& impl();\
	inline const Impl& impl() const;

#define ECO_IMPL_API() \
ECO_IMPL_API_A()\
protected:\
	Impl* m_impl;


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


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
// empty vlaue, using for init.
enum { value_none = 0 };


////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
inline void add(type_t& obj, uint32_t v)
{
	obj |= v;
}
template<typename type_t>
inline void del(type_t& obj, uint32_t v)
{
	obj &= ~v;
}
template<typename type_t>
inline void toggle(type_t& obj, uint32_t v)
{
	obj ^= v;
}
template<typename type_t>
inline void set(type_t& obj, uint32_t v, bool is)
{
	obj = is ? (obj | v) : (obj & ~v);
}
template<typename type_t>
inline void set_v(type_t& obj, const uint32_t add_v, const uint32_t del_v)
{
	eco::add(add_v);
	eco::del(del_v);
}
template<typename type_t>
inline bool has(const type_t& obj, uint32_t v)
{
	return (obj & v) > 0;
}


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
// object id 1: global object.
typedef uint64_t ObjectId;

// object id 2: attached object.
class AspectId
{
public:
	uint64_t object_id;
	uint32_t aspect_id;
};


////////////////////////////////////////////////////////////////////////////////
// general value type
enum
{
	type_bool			= 1,
	type_int32			= 2,
	type_int64			= 3,
	type_string			= 4,
	type_double			= 5,
	type_date			= 6,
	type_time			= 7,
	type_date_time		= 8,
};
typedef uint16_t ValueType;


////////////////////////////////////////////////////////////////////////////////
class Type
{
	ECO_NONCOPYABLE(Type);
public:
	// init.
	inline Type(uint32_t id, const char* name, const Type* parent, void*)
		: m_id(id), m_name(name), m_parent(parent) {}

	// get type id of this type.
	inline const uint32_t id() const { return m_id; }

	// get type name of this type.
	inline const char* name() const { return m_name; }

	// get parent of this type.
	inline const Type* parent() const { return m_parent; }

	// is kind of runtime class.
	inline bool kind_of(IN const Type* clss_sup) const
	{
		const Type* me = this;
		for (; me != nullptr && clss_sup != me; me = me->m_parent) {}
		return (me != nullptr);
	}

private:
	const Type* m_parent;
	const char* m_name;
	const uint32_t m_id;
};
template<typename t>
struct TypeInit
{
	inline TypeInit() { t::type(); }
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif