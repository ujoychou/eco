#ifndef ECO_STD_ATOMIC_H
#define ECO_STD_ATOMIC_H
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
#ifdef ECO_NO_STD_ATOMIC
#include <boost/atomic/atomic.hpp>
ECO_NS_BEGIN(eco);
typedef boost::atomic_int32_t   std_atomic_int32_t;
typedef boost::atomic_int64_t   std_atomic_int64_t;
typedef boost::atomic_uint32_t  std_atomic_uint32_t;
typedef boost::atomic_uint64_t  std_atomic_uint64_t;
typedef boost::atomic<size_t>   std_atomic_size_t;
ECO_NS_END(eco);
#else
#include <atomic>
ECO_NS_BEGIN(eco);
typedef std::atomic<int32_t>    std_atomic_int32_t;
typedef std::atomic<int64_t>    std_atomic_int64_t;
typedef std::atomic<uint32_t>   std_atomic_uint32_t;
typedef std::atomic<uint64_t>   std_atomic_uint64_t;
typedef std::atomic<size_t>     std_atomic_size_t;
ECO_NS_END(eco);
#endif


////////////////////////////////////////////////////////////////////////////////
#endif