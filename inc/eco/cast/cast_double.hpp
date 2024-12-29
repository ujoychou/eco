#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-09-04] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/macro.hpp>
#include <eco/type/number.hpp>
#include <eco/cast/cast_integer.hpp>


eco_namespace(eco);
eco_namespace(cast_detail);
////////////////////////////////////////////////////////////////////////////////
template<typename float_t>
struct diyfp
{
    typedef eco::ieee_754<float_t> ieee_754_t;

    // binary:  v = (+/-) f * 2^e;
    int32_t  s;     // sign
    int32_t  e;     // exponent
    uint64_t f;     // significand
    
    inline diyfp(double d)
    {
        eco::union_memory<double, uint64_t> um(d);
        // get sign and remove sign
        this->s = ((um.u.out >> 63) > 0);
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
    inline diyfp<float_t>& normalize()
    {
        // nomalized float
        if (this->e != 0)
        {
            this->f += ieee_754_t::hide_f;
            this->e -= ieee_754_t::bias_all;
            return *this;
        }
        // denomalized float
        this->e = 1 - ieee_754_t::bias_all;
        return *this;
    }

    inline static void boundary(diyfp& m_plus, diyfp& m_minus)
    {
        /* get_boundary: m-, m+
        ---+-------------+-------------+-------------+-------------+--- [A]
           v-            m-            v             m+            v+
        -----------------+------+------+-------------+-------------+--- [B]
                         v-     m-     v             m+            v+
        
        [A]: when v != 1 * 2^e
        ---------------------------------------------------------------
        m- = (v- + v) / 2 = (f-1 + f) * 2^(e-1) = (2f - 1) * 2^(e-1)
        m- delta = (v - m-) = 2^(e-1)
        m+ = (v+ + v) / 2 = (f+1 + f) * 2^(e-1) = (2f + 1) * 2^(e-1)
        m+ delta = (m+ - v) = 2^(e-1)

        [B]: when v  = 1 * 2^e
        ---------------------------------------------------------------
        v- = (2f - 1) * 2^(e-1)
        m- = (v- + v) / 2 = (2f - 1 + 2f) * 2^(e-1) / 2 = (4f - 1) * 2^(e-2)
        m- delta = 2^(e-2)
        */
        m_plus.f  = (m_plus.f << 1) + 1;
        m_plus.e -= 1;

        if (m_minus.f == ieee_754_t::hide_f)
        {
            // [B]
            m_minus.f  = (m_minus.f << 2) - 1;
            m_minus.e -= 2;
        }
        else
        {
            // [A]
            m_minus.f  = (m_minus.f << 1) - 1;
            m_minus.e -= 1;
        }
    }

    inline static void boundary_delta(diyfp& mp_delta, diyfp& mm_delta)
    {
        // [A]
        mp_delta.f  = 1;
        mp_delta.e -= 1;
        mm_delta.f  = 1;
        mm_delta.e -= 1;
        // [B]
        if (mm_delta.f == ieee_754_t::hide_f)
        {
            mm_delta.e -= 1;
        }
    }

    inline uint32_t exponent_align_right()
    {
        uint32_t count = 0; 
        while (!(this->f & 1))
        {
            this->f >>= 1;
            this->e  += 1;
            count += 1;
        }
        return count;
    }
    inline uint32_t exponent_align_left()
    {
        uint32_t count = 0;
        uint64_t e_align = (ieee_754_t::hide_f << 1);
        while (!(e_align & this->f))
        {
            this->f <<= 1;
            this->e  -= 1;
            count += 1;
        }
        return count;
    }
    inline void exponent_align(int32_t e_align)
    {
        this->f <<= (this->e - e_align);
        this->e = e_align;
    }

    inline bool operator<=(const diyfp& fp) const
    {
        diyfp tmp = fp;
        tmp.exponent_align(this->e);
        return this->f <= tmp.f;
    }
};


////////////////////////////////////////////////////////////////////////////////
struct string_result_double : public eco::cast_detail::string_result
{
public:
	inline string_result_double()
	{
		// double in [2^-59, 2^64), decimal digits size max = 20
		// = 32 - 1 - 20 = capacity - '\0' - max
		string_result::pos = 11;
	}

	inline void push(uint64_t i)
	{
        eco::integer_to_string_decimal dec(*this);
        
		// if i is integer part
		if (pos_start == string_result::capacity)
		{
			dec.cast_10_back(static_cast<uint32_t>(i));
            return;
		}
    
        // = 1 + 20 = sign(+/-) + integer max(2^64)
        if (i > 99'9999'9999ull) { string_result::pos = 21; }
        pos_dot = pos_start = string_result::pos;

        // push cast integer value
        if (i == 0)
        {
            string_result::pos += 1;
            return;
        }
        dec.cast(i);
        pos_start = string_result::pos;
        string_result::pos = pos_dot + 1;
	}

	inline void pretty()
	{
	}
	
private:
	uint32_t pos_start = string_result::capacity;
    uint32_t pos_dot   = string_result::capacity;
};
eco_namespace_end(cast_detail);


////////////////////////////////////////////////////////////////////////////////
class double_to_string_format
{
public:
	inline double_to_string_format(int prec, int16_t mode, int16_t percent)
		: precision(prec), precision_mode(mode), percent(percent)
	{}

	inline eco::bool_t digit_precision() const
	{
		return precision_mode == 1;
	}

    inline eco::bool_t total_precision() const
	{
		return !digit_precision();
	}
    
public:
	int32_t precision;
    int16_t precision_mode;
    int16_t percent;
};


////////////////////////////////////////////////////////////////////////////////
template<typename float_t>
class double_to_string
{
public:
    typedef eco::ieee_754<float_t> ieee_754_t;
    typedef eco::cast_detail::diyfp<float_t> diyfp_t;

    inline double_to_string(cast_detail::string_result_double& r)
        : result(r)
	{}

    inline void pretty(const double_to_string_format& f)
    {
        // prec = -1: means default precision
        if (f.precision == -1) { return; }
        result.pretty();
    }

    inline void operator()(double value, const double_to_string_format& f)
	{
        diyfp_t v(value);
        if (v.sign())
        {
            result.push_back('-');
        }

        if (v.zero())
        {
            result.push_back("0.0");
            pretty(f);
        }
        else if (v.inf())
        {
            result.push_back("inf");
        }
        else if (v.nan())
        {
            result.push_back("nan");
        }
        else
        {
            grisu2(v.normalize(), value);
            pretty(f);
        }
        return *this;
    }

protected:
    inline void grisu2(diyfp_t v, double value)
    {
        eco::cast_detail::diyfp<float_t> mp(v);
        eco::cast_detail::diyfp<float_t> mm(v);
        eco::cast_detail::diyfp<float_t>::boundary_delta(mp, mm);

        // value must be in [2^-59, 2^64)
        if (value < 0) { value = -value; }
        const double max = 1ull << 64;
        const double min = 1.0 / (1ull << 59);
        if (value < max && value >= min)
        {
            uint32_t shift = 0 - v.e;
            do
            {
                uint64_t i = (v.f >> shift);
                v.f  &= (uint64_t(1) << shift) - 1;
                v.f  *= 10;
                mm.f *= 10;
                result.push(i);
            } while (mm <= v);
        }
    }

private:
    eco::cast_detail::string_result_double& result;
};


////////////////////////////////////////////////////////////////////////////////
class string_to_double : public eco::cast_detail::result
{
public:
    inline string_to_double(const eco::string_view& s)
    {
        value = strtod(s.c_str(), &end);
    }


    double value;
    char* end;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);
