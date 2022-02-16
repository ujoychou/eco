#ifndef ECO_STD_THREAD_H
#define ECO_STD_THREAD_H
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
#ifdef ECO_NO_STD_THREAD
#include <boost/thread/thread.hpp>
ECO_NS_BEGIN(eco);
typedef boost::thread std_thread;
namespace std_this_thread = boost::this_thread;
ECO_NS_END(eco);
#else
#include <thread>
ECO_NS_BEGIN(eco);
typedef std::thread std_thread;
namespace std_this_thread = std::this_thread;
ECO_NS_END(eco);
#endif


////////////////////////////////////////////////////////////////////////////////
#endif