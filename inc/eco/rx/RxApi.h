#ifndef ECO_RX_API_H
#define ECO_RX_API_H
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.

@ ujoy modifyed on 2021-11-20
1.Export.h -> RxExport.h


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include <eco/rx/RxVersion.h>


////////////////////////////////////////////////////////////////////////////////
#ifndef ECO_API
// linux so.
#ifdef ECO_LINUX
#ifdef ECO_EXPORT
#	define ECO_API __attribute__(visibility("default"))
#endif
#endif

// windows dll.
#ifdef ECO_WIN
#ifdef ECO_EXPORT
#	define ECO_API __declspec(dllexport)
#else
#	define ECO_API __declspec(dllimport)
#	ifndef ECO_AUTO_LINK_NO
//#		pragma comment(lib, ECO_LIB_NAME(eco))
#	endif
#endif
#endif
#endif // ECO_API


////////////////////////////////////////////////////////////////////////////////
#ifndef ECO_ERX_API
// linux so.
#ifdef ECO_LINUX
#ifdef ECO_EXPORT
#	define ECO_ERX_API __attribute__(visibility("default"))
#endif
#endif

// windows dll.
#ifdef ECO_WIN
#ifdef ECO_EXPORT
// load erx api dynamically, not using dll hint import.
#	define ECO_ERX_API __declspec(dllexport)
#endif
#endif
#endif // ECO_ERX_API

////////////////////////////////////////////////////////////////////////////////
#endif