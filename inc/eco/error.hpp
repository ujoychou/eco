#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-09-04] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/prec.hpp>
#include <eco/string.hpp>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// return error mode: set error information in sub function
#define eco_error(...) eco::error(##__VA_ARGS__, NULL)

// throw error mode: throw error(a spice) object in sub function
#define eco_throw(...) throw eco::error(##__VA_ARGS__, NULL)

////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);