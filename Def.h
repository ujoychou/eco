#ifndef ECO_PREC_H
#define ECO_PREC_H
/*******************************************************************************
@ name
precompile define.

@ function
1.compile option: os/compiler/c++.
2.compile function: code-note
3.compile function: protobuf/noncopable
4.compile function: object/value
5.compile function: singleton

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.

@ ujoy modifyed on 2021-11-20
1.refactory responsibility of base frame.
2.change Export.h -> Prec.h


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include <cstdint>
#include <iostream>


////////////////////////////////////////////////////////////////////////////////
// forbid win define min/max
#undef min
#undef max

// namespace.
namespace eco{}
#define ECO_NS_BEGIN(ns) namespace ns{
#define ECO_NS_END(ns) }

// compile option: operation system.
#ifdef WIN32
#	define ECO_WIN32
#	define ECO_WIN
#endif
#ifdef ECO_WIN32
// windows related header file.
#	include <SDKDDKVer.h>
#endif
#ifdef ECO_LINUX
#	define FCALL
#endif

// compile option: compiler.
#ifdef ECO_VC100
#define ECO_NO_VARIADIC_TEMPLATE
#define ECO_NO_FUNCTION_TEMPLATE_DEFAULT
#define ECO_NO_STD_THREAD
#define ECO_NO_STD_CHRONO
#define ECO_NO_STD_MOVE
#define ECO_PROTOBUF2
#define __func__ eco::func(__FUNCTION__)
#define thread_local __declspec(thread)
#endif

// compile option: c++
#ifdef ECO_NO_CXX11
#	define nullptr (NULL)
#	define override
#endif
#ifndef FCALL
#	define FCALL __cdecl
#endif


////////////////////////////////////////////////////////////////////////////////
// compile function: code note.
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

// compile function: protobuf.
#ifdef ECO_NO_PROTOBUF
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
// compile object: noncopyable.
#define ECO_NONCOPYABLE(type_t) \
private:\
	type_t(IN const type_t& );\
	type_t& operator=(IN const type_t& );

////////////////////////////////////////////////////////////////////////////////
// compile object: single object delare.
#define ECO_OBJECT_PTR(object_t) \
public:\
	typedef object_t object;\
	typedef std::shared_ptr<object_t> value;\
	typedef std::shared_ptr<object_t> ptr;\
	typedef std::weak_ptr<object_t> wptr;
#define ECO_OBJECT(object_t) \
ECO_OBJECT_PTR(object_t) ECO_NONCOPYABLE(object_t);
#define ECO_NEW(object_t) object_t::ptr(new object_t)

// singleton: single object declare.
#define ECO_SINGLETON_1(object_t)\
	ECO_NONCOPYABLE(object_t);\
public:\
	inline static object_t& get() {\
		return eco::Singleton<object_t>::get();\
	}\
private:\
	friend class eco::Singleton<object_t>;\
	inline object_t() {}

#define ECO_SINGLETON_2(object_t, init)\
	ECO_NONCOPYABLE(object_t);\
public:\
	inline static object_t& get() {\
		return eco::Singleton<object_t>::get();\
	}\
private:\
	friend class eco::Singleton<object_t>;\
	inline object_t() { init(); }
#define ECO_SINGLETON(...) ECO_MACRO(ECO_SINGLETON_,__VA_ARGS__)

// singleton: single api.
#define ECO_SINGLETON_API(object_t)\
    ECO_IMPL_API();\
    ECO_NONCOPYABLE(object_t);\
public:\
	~object_t();\
private:\
	friend class eco::Singleton<object_t>;\
	object_t();\

// singleton: single func.
#define ECO_SINGLETON_NAME(object_t, method)\
inline object_t& method()\
{\
	return eco::Singleton<object_t>::get();\
}

// singleton: single lazy func.
#define ECO_SINGLETON_LAZY(object_t, method)\
inline object_t& method()\
{\
	static object_t s_object;\
	return s_object;\
}


ECO_NS_BEGIN(eco)
////////////////////////////////////////////////////////////////////////////////
// make object.
template<typename T>
inline void* make(std::auto_ptr<T>& obj)
{
	obj.reset(new T());
	return obj.get();
}
template<typename T>
inline void* make(std::unique_ptr<T>& obj)
{
	obj.reset(new rsp_t());
	return obj.get();
}
template<typename T>
inline void* make(std::shared_ptr<T>& obj)
{
	obj = std::make_shared<T>();
	return obj.get();
}
template<typename T>
inline void* make(T& obj)
{
	return &obj;
}
template<typename T>
inline void* make(T* obj)
{
	return new T();
}


////////////////////////////////////////////////////////////////////////////////
// get object type.
template<typename T>
inline T& get_object() { return *(T*)nullptr; }
template<typename T>
inline T& get_object(T& obj) { return obj; }
template<typename T>
inline T& get_object(std::shared_ptr<T>& ptr) { return *ptr; }
template<typename T>
inline const T& object(const T& obj) { return obj; }
template<typename T>
inline const T& object(const std::shared_ptr<T>& ptr) { return *ptr; }


////////////////////////////////////////////////////////////////////////////////
// get T when type is shared_ptr<T>, else return T when type is T.
template<typename T>
class Raw
{
public:
	typedef decltype(&eco::get_object(eco::get_object<T>())) ptr;
	typedef decltype(&eco::get_object(eco::get_object<T>())) pointer;
	typedef decltype(eco::get_object(eco::get_object<T>()))  reference;
	typedef decltype(&eco::object(eco::get_object<T>())) const_ptr;
	typedef decltype(&eco::object(eco::get_object<T>())) const_pointer;
	typedef decltype(eco::object(eco::get_object<T>()))  const_reference;
};

template<typename value_t>
class Value
{
public:
	typedef value_t object;
	typedef value_t value;
};

template<typename object_t>
class Object
{
public:
	typedef object_t object;
	typedef std::shared_ptr<object_t> value;
	typedef std::shared_ptr<object_t> ptr;
	typedef std::weak_ptr<object_t> wptr;

protected:
	Object() {};
	~Object() {};

private:
	Object(const Object&);
	const Object& operator=(const Object& );
};

template<typename Object>
class Movable
{
public:
	inline Movable()
	{}

	inline Movable(IN Object& v)
		: m_object(std::move(v))
	{}

	inline Movable(IN Movable&& v)
		: m_object(std::move(v.m_object))
	{}

	inline Movable(IN const Movable& v)
		: m_object(std::move(v.m_object))
	{}

	inline Movable& operator=(IN const Movable& v)
	{
		m_object = std::move(v.m_object);
		return *this;
	}

	inline operator Object& ()
	{
		return m_object;
	}

	inline operator const Object& () const
	{
		return m_object;
	}

private:
	mutable Object m_object;
};

template<typename Object>
inline eco::Movable<Object> move(IN Object& obj)
{
	return eco::Movable<Object>(obj);
}


////////////////////////////////////////////////////////////////////////////////
// singleton proxy object that instantiate the object.
template<typename object_t>
class Singleton
{
	ECO_OBJECT(Singleton);
public:
	inline static object_t& get()
	{
		return s_object;
	}
private:
	static object_t s_object;
};
template<typename object_t>
object_t Singleton<object_t>::s_object;


////////////////////////////////////////////////////////////////////////////////
class Cout
{
public:
	template<typename value_t>
	inline Cout& operator<<(IN const value_t& val)
	{
		std::cout << val;
		return (*this);
	}

	inline ~Cout()
	{
		for (uint8_t i = 0; i < m_turn_line; ++i)
		{
			std::cout << std::endl;
		}
	}

	inline explicit Cout(IN const uint8_t turn_line) : m_turn_line(turn_line)
	{}

private:
	uint8_t m_turn_line;
};
// print stream message to std out auto append std::endl.
inline Cout cout(IN uint8_t turn_line = 1)
{
	return Cout(turn_line);
}
// when app exit with exception, use _getch to show a console windows.
// and show the exception error.
inline char getch_exit()
{
	char ch = 0;
	std::cout << "please input a char to end." << std::endl;
	std::cin >> ch;
	return ch;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif