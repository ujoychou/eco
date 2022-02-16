#ifndef ECO_STD_CHRONO_H
#define ECO_STD_CHRONO_H
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2020-11-27.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2020 - 2025, ujoy, reserved all right.

*******************************************************************************/
#ifdef ECO_NO_STD_CHRONO
#include <boost/chrono/chrono.hpp>
namespace std_chrono = boost::chrono;
#else
#include <chrono>
namespace std_chrono = std::chrono;
#endif

#endif