#ifndef ECO_PROC_H
#define ECO_PROC_H
/*******************************************************************************
@ name
process.

@ function


@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Prec.h>
#include <eco/rx/RxApi.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(this_process);
////////////////////////////////////////////////////////////////////////////////
// get current thread id.
ECO_API size_t id();
ECO_API const char* sid();

////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(this_process);
ECO_NS_END(eco);
#endif