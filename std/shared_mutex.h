#ifndef ECO_STD_SHARED_MUTEX_H
#define ECO_STD_SHARED_MUTEX_H
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
#include <boost/thread/shared_mutex.hpp>
ECO_NS_BEGIN(eco);
typedef boost::shared_mutex shared_mutex;
typedef boost::shared_lock<boost::shared_mutex> reader_lock;
typedef boost::unique_lock<boost::shared_mutex> writer_lock;
ECO_NS_END(eco);
#else
#include <shared_mutex>
ECO_NS_BEGIN(eco);
typedef std::shared_mutex shared_mutex;
typedef std::shared_lock<std::shared_mutex> reader_lock;
typedef std::unique_lock<std::shared_mutex> writer_lock;
ECO_NS_END(eco);
#endif


////////////////////////////////////////////////////////////////////////////////
#endif