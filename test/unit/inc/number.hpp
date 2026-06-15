#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2025-06-21] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/type/number.hpp>


////////////////////////////////////////////////////////////////////////////////
// psize + 1
static inline uint32_t k1_MIN(uint32_t size)
{
    return size + 1;
}
// 1.5 * psize
static inline uint32_t k1_5(uint32_t size)
{
    return size + (size >> 1);
}
// 1.75 * psize
static inline uint32_t k1_75(uint32_t size)
{
    return k1_5(size) + (size >> 2);
}
// 1.875 * psize
static inline uint32_t k1_875(uint32_t size)
{
    return k1_75(size) + (size >> 3);
}
// 2 * psize - 1
static inline uint32_t k1_MAX(uint32_t size)
{
    return (size << 1) - 1;
}


////////////////////////////////////////////////////////////////////////////////