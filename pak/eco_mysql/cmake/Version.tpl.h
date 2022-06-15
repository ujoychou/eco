#ifndef RX_VERSION_H
#define RX_VERSION_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Prec.h>

// configure eco version.
#define ECO_LIB_MAJOR @EcoMysql_VERSION_MAJOR@
#define ECO_LIB_MINOR @EcoMysql_VERSION_MINOR@
#define ECO_LIB_PATCH @EcoMysql_VERSION_PATCH@
#define ECO_LIB_TWEAK @EcoMysql_VERSION_TWEAK@

// configure eco prec.
#define ECO_LINUX
#define ECO_API
#define ECO_ERX_API
#define ECO_NO_STD_SHARED_MUTEX

////////////////////////////////////////////////////////////////////////////////
#endif
