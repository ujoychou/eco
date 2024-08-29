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
#include <assert.h>


////////////////////////////////////////////////////////////////////////////////
#ifdef __WIN32
#	define eco_win32
#endif
#define eco_linux


////////////////////////////////////////////////////////////////////////////////
#undef eco_namespace
#undef eco_namespace_end
#undef eco_todo
#undef eco_note
#define eco_namespace(ns) namespace ns {
#define eco_namespace_end(ns) }
#define eco_todo(thing)
#define eco_note(thing)

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
eco_namespace(eco)
using Bool = uint32_t;
eco_namespace_end(eco)