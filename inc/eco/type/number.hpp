#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-08-21] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/macro.hpp>
#include <stdint.h>


eco_namespace(eco)
////////////////////////////////////////////////////////////////////////////////
template<typename type_in, typename type_out>
struct union_memory
{
	union
	{
		type_in		in;
		type_out 	out;
	} u;

	inline union_memory(type_in in) : u{in} {}
};


////////////////////////////////////////////////////////////////////////////////
template <typename float_t> struct ieee_754;

template<> struct ieee_754<float>
{
    // 32 = 1(sign) + 8(exponent) + 23(significand)
    static const int32_t  bits      = 32;
    static const int32_t  bits_e    = 8;
    static const int32_t  bits_f    = 23;
    // = 2 ^ (bits_e - 1) - 1, so exponent in (-126, 127)
    static const int32_t  bias_e    = 127;
    // = bias_e + bits_f
    static const int32_t  bias_all  = 149;
    static const uint64_t hide_f  = (uint64_t(1) << bits_f); 
};

template<> struct ieee_754<double>
{
    // 64 = 1(sign) + 11(exponent) + 52(significand)
    static const int32_t  bits      = 64;
    static const int32_t  bits_e    = 11;
    static const int32_t  bits_f    = 52;
    // = 2 ^ (bits_e - 1) - 1, so exponent in (-1022, 1023)
    static const int32_t  bias_e    = 1023;
    // = bias_e + bits_f
    static const int32_t  bias_all  = 1075;
    static const uint64_t hide_f    = (uint64_t(1) << bits_f);
};


////////////////////////////////////////////////////////////////////////////////
// integer
inline bool_t big_endian()
{
	eco::union_memory<uint32_t, char[4]> um(1);
	return um.u.out[0] == 1;
}
inline bool_t little_endian()
{
	return !big_endian();
}
inline uint32_t to_base_shift(uint32_t base)
{
	uint32_t shift = 1;
	if (base < 2 || (base & (base - 1)) != 0) { return 0; }
	for (; base > 2; base >>= 1, ++shift) {}
	return shift;
}


////////////////////////////////////////////////////////////////////////////////
// double
inline bool_t sign(double v)
{
	eco::union_memory<double, uint64_t> um(v);
	return (um.u.out >> 63) != 0;
}

inline bool_t inf(double v)
{
	eco::union_memory<double, uint64_t> um(v);
	// get exponent and significand
	uint32_t e = (um.u.out << 1) >> (52 + 1);
	uint64_t f = (um.u.out << 12);
	return (e == (uint64_t(1) << 11) - 1) && f == 0;
}

inline bool_t nan(double v)
{
	eco::union_memory<double, uint64_t> um(v);
	// get exponent and significand
	uint64_t e = (um.u.out << 1) >> (52 + 1);
	uint64_t f = (um.u.out << 12);
	return (e == (uint64_t(1) << 11) - 1) && f != 0;
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);