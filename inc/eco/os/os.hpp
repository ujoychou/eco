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
eco_api void* dll_load(const char* path);
eco_api void  dll_free(void*& handle);
eco_api const char* dll_path();
eco_api eco::func_t dll_func(void* handle, const char* func_name);
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(os);
eco_namespace_end(eco);