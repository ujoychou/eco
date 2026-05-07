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
#include <eco/prec.hpp>
#include <eco/type/number.hpp>
#include <eco/cast/cast_integer.hpp>
#include <eco/cast/cast_double_ryu.hpp>


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
// Ryu-based shortest round-trip decimal converter.
template<typename float_t>
class double_to_string
{
public:
    inline double_to_string()
    {
        m_result.buff[0] = 0;
        m_size = 0;
    }

    // Cast a double to its shortest scientific-notation string.
    // Result accessible via c_str() / size() (NUL-terminated).
    inline double_to_string& cast(double v)
    {
        m_size = ryu_detail::d2s_buffered_n(v, m_result.buff);
        m_result.buff[m_size] = '\0';
        m_result.pos = 0;
        return *this;
    }

    inline const char* c_str() const { return m_result.buff; }
    inline uint32_t    size () const { return (uint32_t)m_size; }

public:
    // Lower-level access for users who already manage their own buffer.
    static inline int cast(double v, char* out)
    {
        const int n = ryu_detail::d2s_buffered_n(v, out);
        out[n] = '\0';
        return n;
    }

private:
    eco::cast_detail::result m_result;
    int                      m_size;
};


////////////////////////////////////////////////////////////////////////////////
template<typename float_t>
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
