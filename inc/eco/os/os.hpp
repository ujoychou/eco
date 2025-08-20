#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2025-08-20] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2025 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/prec.hpp>
#include <eco/rtti/api.hpp>


eco_namespace(eco);
eco_namespace(os);
////////////////////////////////////////////////////////////////////////////////
eco_api void* dll_load(const char* dll_name);
eco_api void  dll_free(void*& dll_handle);
eco_api eco::function_t dll_func(void* dll_handle, const char* func_name);
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(os);
eco_namespace_end(eco);