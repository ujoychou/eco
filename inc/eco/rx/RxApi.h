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
#include <eco/Prec.h>
#include <eco/rx/RxVersion.h>


////////////////////////////////////////////////////////////////////////////////
#ifndef ECO_API
// export eco_api
#ifdef ECO_EXPORT
#   ifdef ECO_LINUX
#	    define ECO_API __attribute__(visibility("default"))
#   elif defined ECO_WIN
#	    define ECO_API __declspec(dllexport)
#   endif
// import eco_api
#else
#   ifdef ECO_WIN
#	    define ECO_API __declspec(dllimport)
#	endif
#	ifndef ECO_AUTO_LINK_NO
#		pragma comment(lib, ECO_LIB_NAME(eco))
#	endif
#endif
#endif


////////////////////////////////////////////////////////////////////////////////
#ifndef ECO_ERX_API
// export eco_erx_api
#ifdef ECO_ERX_EXPORT
#   ifdef ECO_LINUX
#	    define ECO_ERX_API __attribute__(visibility("default"))
#   elif defined ECO_WIN
#	    define ECO_ERX_API __declspec(dllexport)
#   endif
// import eco_erx_api
#else
#   ifdef ECO_WIN
#	    define ECO_ERX_API __declspec(dllimport)
#	endif
#	ifndef ECO_AUTO_LINK_NO
#		pragma comment(lib, ECO_LIB_NAME(eco))
#	endif
#endif
#endif


////////////////////////////////////////////////////////////////////////////////
#endif
