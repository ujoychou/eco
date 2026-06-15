#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-08-21] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <memory>


////////////////////////////////////////////////////////////////////////////////
// platform: os
#ifdef __WIN32
#	define eco_win32
#else
#   define eco_linux
#endif

// platform: complier
#ifdef __clang__
#   define eco_clang
#elif defined (__GNUC__)
#   define eco_gnuc
#endif


////////////////////////////////////////////////////////////////////////////////
#if defined(eco_gnuc) || defined(eco_clang)
#  define eco_likely(x)     __builtin_expect(!!(x), 1)
#  define eco_unlikely(x)   __builtin_expect(!!(x), 0)
#else
#  define eco_likely(x)     (x)
#  define eco_unlikely(x)   (x)
#endif

////////////////////////////////////////////////////////////////////////////////
// c++ namespace
#define eco_namespace(ns) namespace ns {
#define eco_namespace_end(ns) }
#define eco_todo(thing)
#define eco_note(thing)

// c++ macro override
#define eco_macro_cat_(a, b) a##b
#define eco_macro_cat(a, b) eco_macro_cat_(a, b)
#define eco_macro_str_(a) #a
#define eco_macro_str(a) eco_macro_str_(a)
#define eco_macro_getn_(n0,n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12,n13,n14,n15,\
n16,n17,n18,n19,n20,n21,n22,n23,n24,n25,n26,n27,n28,n29,n30,n31,size,...) size
#define eco_macro_getn(...) eco_macro_cat(eco_macro_getn_(__VA_ARGS__,32,31,30,\
29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,),)
#define eco_macro_overload(macro, ...) \
eco_macro_cat(macro, eco_macro_getn(__VA_ARGS__))(__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
// c++ typedef
eco_namespace(eco)
using bool_t = uint32_t;
using func_t = void (*)(void);
using offset_t = uint32_t;
const uint32_t U32_1 = static_cast<uint32_t>(-1);
enum class result : int
{
    ok 	        = 0,
    fail        = 1,
    error	    = 2,
    timeout     = 3,
	defined     = 1000,
};


////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
class value_t
{
public:
	typedef type_t ref;
	typedef type_t value;
};


////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
class object_t
{
public:
	typedef type_t value;
	typedef std::shared_ptr<type_t> ref;
	typedef std::shared_ptr<type_t> ptr;
	typedef std::weak_ptr<type_t> wptr;

protected:
	inline object_t() {};
	inline ~object_t() {};

private:
	inline object_t(const object_t&);
	inline const object_t& operator=(const object_t& );
};

// compile object: noncopyable.
#define eco_noncopyable(type_t) \
private:\
	type_t(const type_t& );\
	type_t& operator=(const type_t& );

// compile object: single object delare.
#define eco_object_ptr(object_t) \
public:\
	typedef std::shared_ptr<object_t> value;\
	typedef std::shared_ptr<object_t> ptr;\
	typedef std::weak_ptr<object_t> wptr;
#define eco_object(object_t) \
eco_object_ptr(object_t) \
eco_noncopyable(object_t);


////////////////////////////////////////////////////////////////////////////////
// singleton proxy object that instantiate the object.
template<typename type_t>
class singleton
{
	eco_noncopyable(singleton);
    static type_t s_get;
public:
	inline static type_t& get() { return s_get; }
};
template<typename type_t>
type_t singleton<type_t>::s_get;

// singleton: single object declare.
#define eco_singleton(type_t)\
	eco_noncopyable(type_t);\
    friend class eco::singleton<type_t>;\
    inline type_t() {}\
public:\
	inline static type_t& get() { return eco::singleton<type_t>::get(); }


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco)