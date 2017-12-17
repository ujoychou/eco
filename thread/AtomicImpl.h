#ifndef ECO_ATOMIC_IMPL_H
#define ECO_ATOMIC_IMPL_H
/*******************************************************************************
@ name
atomic sync.

@ function

@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#ifdef ECO_WIN
#	include <windows.h>
#endif







namespace eco{;
namespace atomic{;

#ifdef ECO_WIN
inline int32_t exchange_add(volatile int32_t* v, const int32_t ex)
{
	return ::InterlockedExchangeAdd((uint32_t*)(v), ex);
}
inline uint32_t exchange_add(volatile uint32_t* v, const uint32_t ex)
{
	return ::InterlockedExchangeAdd(v, ex);
}
inline int64_t exchange_add(volatile int64_t* v, int64_t ex)
{
	return ::InterlockedExchangeAdd64(v, ex);
}
inline uint64_t exchange_add(volatile uint64_t* v, int64_t ex)
{
	return ::InterlockedExchangeAdd64((int64_t*)(v), ex);
}
#endif

}}// ns



////////////////////////////////////////////////////////////////////////////////
#endif