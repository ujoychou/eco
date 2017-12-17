#ifndef ECO_EXPORT_API_H
#define ECO_EXPORT_API_H
/*******************************************************************************
@ name
dll export api.

@ function
1.support windows and linux api export.

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include "Export.h"


////////////////////////////////////////////////////////////////////////////////
#if defined(ECO_LINUX)
#	define ECO_API
#	define ECO_EXPORT_API
#endif

#ifndef ECO_API
#	ifdef ECO_EXPORT
#		define ECO_API  __declspec(dllexport)
#	else
#		define ECO_API  __declspec(dllimport)
#	endif
#endif

#ifndef ECO_EXPORT_API
#	define ECO_EXPORT_API  __declspec(dllexport)
#endif

////////////////////////////////////////////////////////////////////////////////
#ifndef ECO_AUTO_LINK_NO
#	if !defined(ECO_EXPORT)
#		pragma comment(lib, "eco.lib")
#	endif
#endif

////////////////////////////////////////////////////////////////////////////////
#endif