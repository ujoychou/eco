#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1. create and init this class.

@ ujoy modifyed on 2021-11-20
1. Export.h -> RxExport.h

@ ujoy modifyed on 2025-08-14
1. RxApi.h -> rxapi.h
2. move eco_lib_name macro from macro.hpp to here.

--------------------------------------------------------------------------------
* copyright(c) 2016 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include <eco/prec.hpp>
#include <eco/export/version.hpp>


////////////////////////////////////////////////////////////////////////////////
// eco lib name define.
#define eco_lib_name_cat(name, major, minor, patch) \
eco_lib_name_cat_(name, major, minor, patch)
#define eco_lib_name_cat_(name, major, minor, patch) \
eco_macro_str(name.major.minor.patch)

// linux lib: "libeco.so.2.3.0"
#ifdef eco_linux
#   define eco_lib_name(name) \
    eco_lib_name_cat(lib##name.so, name##_lib_major, \
                     name##_lib_minor, name##_lib_patch)
// windows lib: "eco.lib.2.3.0"
#elif defined eco_win32
#   define eco_lib_name(name) \
    eco_lib_name_cat(name.lib, name##_lib_major, \
                     name##_lib_minor, name##_lib_patch)
#endif


////////////////////////////////////////////////////////////////////////////////
#ifndef eco_api
// export eco_api
#ifdef eco_export
#   ifdef eco_linux
#	    define eco_api __attribute__(visibility("default"))
#   elif defined eco_win32
#	    define eco_api __declspec(dllexport)
#   endif
// import eco_api
#else
#   ifdef eco_win32
#	    define eco_api __declspec(dllimport)
#	endif
#	ifndef eco_not_auto_link
#		pragma comment(lib, eco_lib_name(eco))
#	endif
#endif
#endif


////////////////////////////////////////////////////////////////////////////////
#ifndef eco_erx_api
// export eco_erx_api
#ifdef eco_erx_export
#   ifdef eco_linux
#	    define eco_erx_api __attribute__(visibility("default"))
#   elif defined eco_win32
#	    define eco_erx_api __declspec(dllexport)
#   endif
// import eco_erx_api
#else
#   ifdef eco_win32
#	    define eco_erx_api __declspec(dllimport)
#	endif
#	ifndef eco_not_auto_link
#		pragma comment(lib, eco_lib_name(eco))
#	endif
#endif
#endif
