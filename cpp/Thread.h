#ifndef ECO_CPP_THREAD_H
#define ECO_CPP_THREAD_H
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
#include <boost/atomic/atomic.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread.hpp>
ECO_NS_BEGIN(eco);
namespace std_this_thread = boost::this_thread;
typedef boost::atomic_int64_t std_atomic_int64_t;
typedef boost::atomic_uint32_t std_atomic_uint32_t;
typedef boost::mutex std_mutex;
typedef boost::thread std_thread;
typedef boost::mutex::scoped_lock std_lock_guard;
typedef boost::unique_lock<boost::mutex> std_unique_lock;
typedef boost::condition_variable std_condition_variable;
typedef boost::shared_mutex shared_mutex;
typedef boost::shared_lock<boost::shared_mutex> reader_lock;
typedef boost::unique_lock<boost::shared_mutex> writer_lock;
ECO_NS_END(eco);
#else
#include <mutex>
#include <atomic>
#include <thread>
#include <shared_mutex>
ECO_NS_BEGIN(eco);
namespace std_this_thread = std::this_thread;
typedef std::atomic_int64_t std_atomic_int64_t;
typedef std::atomic_uint32_t std_atomic_uint32_t;
typedef std::mutex std_mutex;
typedef std::thread std_thread;
typedef std::lock_guard<std::mutex> std_lock_guard;
typedef std::unique_lock<std::mutex> std_unique_lock;
typedef std::condition_variable std_condition_variable;
typedef std::shared_mutex shared_mutex;
typedef std::shared_lock<std::shared_mutex> reader_lock;
typedef std::unique_lock<std::shared_mutex> writer_lock;
ECO_NS_END(eco);
#endif


////////////////////////////////////////////////////////////////////////////////
#endif