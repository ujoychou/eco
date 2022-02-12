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
#include <cstddef>
#include <cstdint>


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
#define ECO_MACRO_GETN(...) ECO_MACRO_CAT(ECO_MACRO_GETN_(__VA_ARGS__,32,\
31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,\
3,2,1,),)
#define ECO_MACRO(macro, ...) \
ECO_MACRO_CAT(macro, ECO_MACRO_GETN(__VA_ARGS__))(__VA_ARGS__)

// lib name: exp "eco.lib.2.3.0"
#define ECO_LIB_NAME_CAT(name, major, minor, patch) \
ECO_LIB_NAME_CAT_(name, major, minor, patch)
#define ECO_LIB_NAME_CAT_(name, major, minor, patch) \
ECO_MACRO_STR(name.lib.major.minor.patch)
#define ECO_LIB_NAME(name) \
ECO_LIB_NAME_CAT(name, ECO_LIB_MAJOR, ECO_LIB_MINOR, ECO_LIB_PATCH)


////////////////////////////////////////////////////////////////////////////////
#ifdef ECO_WIN32
inline int snprintf(OUT char* buf, IN size_t size, IN const char* format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	int result = _vsnprintf(buf, size, format, arglist);
	va_end(arglist);
	return result;
}
#endif


////////////////////////////////////////////////////////////////////////////////
#endif