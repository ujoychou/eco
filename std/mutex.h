#ifndef ECO_STD_MUTEX_H
#define ECO_STD_MUTEX_H
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
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_types.hpp>
ECO_NS_BEGIN(eco);
typedef boost::mutex std_mutex;
typedef boost::mutex::scoped_lock std_lock_guard;
typedef boost::unique_lock<boost::mutex> std_unique_lock;
ECO_NS_END(eco);
#else
#include <mutex>
ECO_NS_BEGIN(eco);
typedef std::mutex std_mutex;
typedef std::lock_guard<std::mutex> std_lock_guard;
typedef std::unique_lock<std::mutex> std_unique_lock;
ECO_NS_END(eco);
#endif


////////////////////////////////////////////////////////////////////////////////
#endif