#ifndef RX_VERSION_H
#define RX_VERSION_H
////////////////////////////////////////////////////////////////////////////////
#include <eco/Prec.h>

// configure eco version.
#define ECO_UTT_LIB_MAJOR @EcoUtt_VERSION_MAJOR@
#define ECO_UTT_LIB_MINOR @EcoUtt_VERSION_MINOR@
#define ECO_UTT_LIB_PATCH @EcoUtt_VERSION_PATCH@
#define ECO_UTT_LIB_TWEAK @EcoUtt_VERSION_TWEAK@

// configure eco prec.
#define ECO_LINUX
#define ECO_API
#define ECO_ERX_API
#define ECO_NO_STD_SHARED_MUTEX

////////////////////////////////////////////////////////////////////////////////
#endif