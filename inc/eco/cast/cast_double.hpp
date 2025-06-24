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
////////////////////////////////////////////////////////////////////////////////
class double_format
{
public:
    static const int mode_e = 1;        // sicentific
    static const int mode_f = 2;        // fixedpoint
    static const int mode_g = 3;        // auto
	/*
	fixedpoint: precision(-1): 1234.5
	fixedpoint, precision(0): 1235
	fixedpoint, precision(2): 1234.50
	fixedpoint, precision(2), percent(1): 123450%
	sicentific: precision(-1): 1.2345e3;
	sicentific, precision(0): 1e3
	sicentific, precision(2): 1.23e3
	sicentific, precision(2), percent(1): 1.23e5%
	*/
	inline double_format(int precision, int mode, bool_t percent)
        : mode(static_cast<uint16_t>(mode))
		, percent(static_cast<uint16_t>(percent))
        , precision(precision)
    {}

    inline double precision_delta() const
    {
        return 1.0 / (1 << precision);
    }
    
public:
	int16_t mode;
    int16_t percent;
    int32_t precision;
};


eco_namespace(cast_detail);
////////////////////////////////////////////////////////////////////////////////
template<typename float_t>
struct diyfp : public ieee_754_value<float_t>
{
public:
    inline diyfp() : ieee_754_value<float_t>() {}

    inline diyfp(float_t v) : ieee_754_value<float_t>(v) {}

    inline diyfp(int32_t e, ieee_754_value<float_t>::uint_t f) 
        : ieee_754_value<float_t>(e, f) 
    {}

    inline void exponent_align(int32_t e_align)
    {
        int32_t shift = this->e - e_align;
        this->f = (shift >= 0)  ? (this->f << shift) : (this->f >> shift);
        this->e = e_align;
    }

    inline bool operator<(const diyfp& fp) const
    {
        diyfp tmp = fp;
        tmp.exponent_align(this->e);
        return this->f < tmp.f;
    }
    
    inline diyfp operator+(const diyfp& fp) const
    {
        diyfp tmp = fp;
        tmp.exponent_align(fp.e);
        tmp.f += this->f;
        return tmp;
    }
};

////////////////////////////////////////////////////////////////////////////////
struct double_to_string_result
{
public:
	inline double_to_string_result(
        const eco::double_format& f,
        eco::cast_result& r)
        : m_format(f), m_result(r)
	{
		m_result.pos = 11;
	}

    inline void set_integer(uint64_t i)
    {
        // double in [2^-59, 2^64), decimal digits size max = 20
		// 11 = 32 - 1 - 20 = capacity - '\0' - max
        // 21 = 1 + 20 = sign(+/-) + integer max(2^64)
        eco::integer_to_string_decimal dec(m_result);
        m_result.pos = (i > 99'9999'9999ull) ? 21 : 11;
        m_pos_dot = m_pos_start = m_result.pos;
        dec.cast(i);
        m_pos_start = m_result.pos;
        m_result.pos = m_pos_dot;

        // 12345 => 1.2345e4
        if (m_format.mode == double_format::mode_e)
        {
            m_pos_start -= 1;
            m_result.buff[m_pos_start] = m_result.buff[m_pos_start + 1];
            m_result.buff[m_pos_start + 1] = '.';
        }
        // 12345 => 1234500%
        else if (m_format.percent)
        {
            m_result.pos = ;
        }
    }

	inline void add_decimal(uint32_t i)
	{
        eco::integer_to_string_decimal dec(m_result);
        dec.cast_10_back(i);
	}

    inline int exponent_length(int exponent)
    {
        // double's decimal exponent in [-304, 308], exp: e-304, e308
        int length = 1;
        if (exponent < 0) { exponent = -exponent; ++length; }

        if (exponent < 10)
            length += 1;
        else if (exponent < 100)
            length += 2;
        else
            length += 3;
        return length;
    }

    inline void format_fixedpoint(uint32_t precision, bool_t percent)
    {
        // exp: 234.0 => 234
        if (m_result.pos == m_pos_dot + 1)
        {
            m_result.pos = m_pos_dot;
        }
        else
        {
            m_result.buff[m_pos_dot] = '.';
        }   
        m_result.buff[m_result.pos] = 0;
    }

    inline void format_scientific()
    {
        // exp: 23400000 => 2.34e7
        // exp: 0.000234 => 2.34e-4
        // exp: 1000.234 => 1.000234e3
    }

	inline void pretty()
	{
        // exp: 234.0 => 234
        // exp: 2340000 => 2.34e6
        int sci_format_len = 0;
        int digit_format_len = (m_result.pos - m_pos_start);
        if (m_result.pos == m_pos_dot + 1)
        {
            m_result.pos = m_pos_dot;
            // exp: 2340000 => 2.34e6
            uint32_t i = m_pos_dot - 1;
            for (; i > m_pos_start && m_result.buff[i] == '0'; --i);
            if (i != m_pos_dot - 1)
            {
                // number length: 2.34
                sci_format_len = (i + 1 - m_pos_start) + 1;
                // exponent length: e6
                exponent = m_pos_dot - m_pos_start - 1;
                int exponent_length = exponent_lenth(exponent);
                sci_format_len += exponent_length;
                m_pos_dot = m_pos_start;
                m_pos_start = m_pos_start - 1;
                m_result.pos = i + exponent_length;
                m_result.buff[m_pos_start] = m_result.buff[m_pos_start + 1];
            }
        }
        // exp: 0.000234 => 2.34e-4
        else if (m_pos_start == m_pos_dot - 1 && m_result.buff[m_pos_start] == '0')
        {
            uint32_t i = m_pos_dot + 1;
            for (; m_result.buff[i] == '0'; ++i);
            // number length: 2.34
            int sci_format_len = m_result.pos - i + 1;
            // exponent length: e-4
            exponent = i - m_pos_dot;
            sci_format_len += exponent_lenth(exponent);
            m_pos_start = i - 1;
            m_pos_dot = i;
            // reset dot pos
            m_result.buff[m_pos_start] = m_result.buff[i];
            m_result.buff[m_result.pos] = 0;
        }

        // if scientific format is shorter than digit format.
        m_result.buff[m_pos_dot] = '.';
        if (sci_format_len < digit_format_len)
        {
            // append exponent to string
            eco::integer_to_string_decimal dec(*this);
            dec.cast(exponent);
            m_result.pos = m_pos_start - 1 + sci_format_len;
        }
        m_result.buff[m_result.pos] = 0;
	}
	
private:
	eco::double_format m_format;
    cast_result&  m_result;
    uint32_t m_pos_start = cast_result::capacity;
    uint32_t m_pos_dot   = cast_result::capacity;
};
eco_namespace_end(cast_detail);


////////////////////////////////////////////////////////////////////////////////
template<typename float_t>
class double_to_string
{
public:
    typedef ieee_754<float_t>::uint_t uint_t;
    typedef eco::cast_detail::diyfp<float_t> diyfp_t;

    inline double_to_string(
        float_t v, 
        const eco::double_format& f,
        eco::cast_result& r)
        : result(f, r)
	{
        diyfp_t diy(v);
        if (diy.zero())
        {
            this->result.push_zero();
            this->result.pretty();
            if (diy.sign()) { this->result.push_front('-'); }
        }
        else if (diy.inf())
        {
            this->result.push_back("inf");
            if (diy.sign()) { this->result.push_front('-'); }
        }
        else if (diy.nan())
        {
            this->result.push_back("nan");
        }
        else if (diyfp_t::range_of_simple(v))
        {
            simple(diy);
            this->result.pretty();
            if (diy.sign()) { this->result.push_front('-'); }
        }
        else
        {
            dragon4(v);
        }
    }

private:
    inline void simple(diyfp_t& diy)
    {
        diy.normalize();
        if (diy.e >= 0)
        {
            this->result.set_integer(diy.f << diy.e);
        }
        else
        {
            diyfp_t mp;
            diyfp_t mm;
            diy.precision(mp, mm);
            uint32_t shift = -diy.e;
            uint_t flags = (uint_t(1) << shift) - 1;
            diyfp_t one(diy.e, uint_t(1) << shift);

            // integer
            ieee_754<float_t>::uint_t i = (diy.f >> shift);
            diy.f &= flags;
            bool_t carry = (one < diy.add(mp));
            bool_t finish = this->result.set_integer(carry + i);

            // decimal
            while (!finish && !carry && mm < diy)
            {
                diy.f *= 10;
                mm.f  *= 10;
                mp.f  *= 10;
                i = (diy.f >> shift);
                diy.f &= flags;
                carry = (one < diy.add(mp));
                finish = this->result.add_decimal(i + carry);
            }
        }
    }

    inline void dragon4(double v)
    {
    }

    eco::cast_detail::double_to_string_result result;
};


////////////////////////////////////////////////////////////////////////////////
class string_to_double
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
