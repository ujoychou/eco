#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2025-08-13] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/log.hpp>


eco_namespace(eco);
eco_namespace(log);
////////////////////////////////////////////////////////////////////////////////
inline const char* name(eco::log::level value)
{
    switch (value)
    {
    case eco::log::debug: return "debug";
    case eco::log::info:  return "info ";
    case eco::log::warn:  return "warn ";
    case eco::log::error: return "error";
    case eco::log::fatal: return "fatal";
    case eco::log::none:  return "none ";
    case eco::log::l1:    return "l1   ";
    case eco::log::l2:    return "l2   ";
    case eco::log::l3:    return "l3   ";
    case eco::log::l4:    return "l4   ";
    case eco::log::l5:    return "l5   ";
    case eco::log::l6:    return "l6   ";
    case eco::log::l7:    return "l7   ";
    case eco::log::l8:    return "l8   ";
    case eco::log::l9:    return "l9   ";
    case eco::log::la:    return "la   ";
    case eco::log::lb:    return "lb   ";
    }
    return "unknown";
}

inline const char* filename(const char* name, int nth)
{
	return "";
}
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);