#ifndef ECO_STD_CONDITION_VARIABLE_H
#define ECO_STD_CONDITION_VARIABLE_H
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
#ifdef ECO_NO_STD_CONDITION_VARIABLE
#include <boost/thread/condition_variable.hpp>
ECO_NS_BEGIN(eco);
typedef boost::condition_variable std_condition_variable;
ECO_NS_END(eco);
#else
#include <condition_variable>
ECO_NS_BEGIN(eco);
typedef std::condition_variable std_condition_variable;
ECO_NS_END(eco);
#endif


////////////////////////////////////////////////////////////////////////////////
#endif