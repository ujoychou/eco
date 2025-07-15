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

    eco::cast_result result;
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
    char*  end;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);
