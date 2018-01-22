#ifndef ECO_PROCESS_H
#define ECO_PROCESS_H
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
#include <eco/ExportApi.h>
#include <process.h>


namespace eco{;
namespace this_process{;


////////////////////////////////////////////////////////////////////////////////
// get current thread id.
size_t get_id();
const char* get_id_string();
}}


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
}
#endif