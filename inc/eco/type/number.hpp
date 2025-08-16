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
#include <eco/prec.hpp>
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

	inline union_memory(type_in in) {}
};


////////////////////////////////////////////////////////////////////////////////
template <typename float_t>
struct ieee_754;

template<> struct ieee_754<float>
{
	typedef uint32_t uint_t;
    // 32 = 1(sign) + 8(exponent) + 23(significand)
    static const int32_t  bits      = 32;
    static const int32_t  bits_e    = 8;
    static const int32_t  bits_f    = 23;
    // = 2 ^ (bits_e - 1) - 1, so exponent in (-126, 127)
    static const int32_t  bias_e    = 127;
    // = bias_e + bits_f
    static const int32_t  bias_all  = 149;
    static const uint64_t hide_f    = (uint64_t(1) << bits_f); 
};

template<> struct ieee_754<double>
{
	typedef uint64_t uint_t;
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

template<typename float_t>
struct ieee_754_value
{
    typedef eco::ieee_754<float_t> ieee_754_t;
    typedef eco::ieee_754_value<float_t> ieee_754_value_t;

    // binary:  v = (+/-) f * 2^e;
    int32_t  s;     // sign
    int32_t  e;     // exponent
    uint64_t f;     // significand
    
    inline ieee_754_value() : s(0), e(0), f(0) {}

    inline ieee_754_value(int32_t e, uint64_t f) :s(0), e(e), f(f) {}
    
    inline ieee_754_value(float_t v)
    {
        eco::union_memory<double, typename ieee_754_t::uint_t> um(v);
        // get sign and remove sign
        this->s = ((um.u.out >> (ieee_754_t::bits - 1)) > 0);
        um.u.out = ((um.u.out << 1) >> 1);
        // get exponent and significand
        this->f = (um.u.out << ieee_754_t::bits_e + 1);
        this->e = (um.u.out >> ieee_754_t::bits_f);
    }

    inline bool_t zero() const
    {
        return this->f == 0 && this->e == 0;
    }

    inline bool_t inf() const
    {
        return this->f == 0 && this->e == (1 << ieee_754_t::bits_e) - 1;
    }

    inline bool_t nan() const
    {
        return this->f != 0 && this->e == (1 << ieee_754_t::bits_e) - 1;
    }
    
    inline bool_t sign() const
    {
        return this->s != 0;
    }

public:
    inline void normalize()
    {
        // nomalized float
        if (this->e != 0)
        {
            this->f += ieee_754_t::hide_f;
            this->e -= ieee_754_t::bias_all;
            return ;
        }
        // denomalized float
        this->e = 1 - ieee_754_t::bias_all;
    }

    inline void precision(ieee_754_value_t& mp, ieee_754_value_t& mm)
    {
        // [A]
        mp.f = mm.f = 1;
        mp.e = mm.e = this->e - 1;  // * 0.5
        // [B]
        if (this->f == ieee_754_t::hide_f)
        {
            mm.e = this->e - 1;     // * 0.5
        }
    }
};
typedef ieee_754_value<float>  float_t;
typedef ieee_754_value<double> double_t;


////////////////////////////////////////////////////////////////////////////////
// integer
template<typename int_t>
inline eco::bool_t signed_int()
{
    return static_cast<int_t>(-1) < 0;
}
inline eco::bool_t big_endian()
{
	eco::union_memory<uint32_t, char[4]> um(1);
	return um.u.out[0] == 1;
}
inline eco::bool_t little_endian()
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
inline uint32_t count_highest_bit(uint64_t n)
{
    // binary search method
    uint32_t pos = 0;
    if (n == 0)         { return pos; }
    if (n & 0xFFFFFFFF00000000) { pos += 32; n >>= 32; }
    if (n & 0xFFFF0000) { pos += 16; n >>= 16; }
    if (n & 0xFF00)     { pos += 8;  n >>= 8;  }
    if (n & 0xF0)       { pos += 4;  n >>= 4;  }
    if (n & 0b1100)     { pos += 2;  n >>= 2;  }
    if (n & 0b10)       { pos += 2;  return pos;  }
    return pos += 1;    // 0b01
}
inline uint32_t clzll(uint64_t n)
{
    if (n == 0) { return 64; }
#if (defined(__GNUC__) && __GNUC__ >= 4)
    return static_cast<uint32_t>(__builtin_clzll(n));
#elif defined(_WIN64)
    uint32_t r = 0;
    _BitScanReverse64(&r, n);
    return 63 - r;
#elif defined(_WIN32)
    uint32_t r = 0;
    if (_BitScanReverse(&r, static_cast<uint32_t>(n >> 32))) { return 31 - r; }
    _BitScanReverse(&r, static_cast<uint32_t>(n & 0xFFFFFFFF));
    return 63 - r;
#else
    return 63 - count_highest_bit(n);
#endif
}
inline uint32_t chbll(uint64_t n)
{
    return 64u - clzll(n);
}
inline eco::bool_t power_of_two(uint64_t align)
{
    return (align & (align - 1)) == 0;
}
inline size_t align_up(size_t size, size_t align = sizeof(void*))
{
    return (size + (align - 1)) & ~(align - 1);
}
inline uint32_t align_power_of_two(uint32_t align)
{
    // if align is a "power of two"
    if (power_of_two(align)) { return align; }

    // if align is not a "power of two", adjust it
    const uint32_t max_align = 256;
    const uint32_t min_align = 4;
    if (align <= min_align) { return min_align; }
    if (align >= max_align) { return max_align; }

    // count_highest_bit
    uint32_t pos = 0;
    if (align & 0b11110000) { pos += 4; align >>= 4; }
    if (align & 0b1100)     { pos += 2; align >>= 2; }
    if (align & 0b10)       { return 1 << (pos + 2); }
    return 1 << (pos + 1);
}
inline uint32_t count_highest_bit_ge(uint32_t value)
{
    return power_of_two(value) ? count_highest_bit(value)
        : count_highest_bit(value) + 1; 
}


////////////////////////////////////////////////////////////////////////////////
// double
inline eco::bool_t sign(double v)
{
	eco::union_memory<double, uint64_t> um(v);
	return (um.u.out >> 63) != 0;
}

inline eco::bool_t inf(double v)
{
	eco::union_memory<double, uint64_t> um(v);
	// get exponent and significand
	uint32_t e = (um.u.out << 1) >> (52 + 1);
	uint64_t f = (um.u.out << 12);
	return (e == (uint64_t(1) << 11) - 1) && f == 0;
}

inline eco::bool_t nan(double v)
{
	eco::union_memory<double, uint64_t> um(v);
	// get exponent and significand
	uint64_t e = (um.u.out << 1) >> (52 + 1);
	uint64_t f = (um.u.out << 12);
	return (e == (uint64_t(1) << 11) - 1) && f != 0;
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);