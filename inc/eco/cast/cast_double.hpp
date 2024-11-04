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
#include <eco/string/string_c.hpp>
#include <eco/string/string_view.hpp>
#include <eco/cast/cast_def.hpp>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
class double_to_string : public eco::cast_detail::string_result
{
public:
    inline bool operator()(double v, int precision, bool_t percent)
	{
        (void)v;
        (void)precision;
        (void)percent;
        return true;
    }
};


////////////////////////////////////////////////////////////////////////////////
class string_to_double
{
public:
    inline string_to_double(const eco::string_view& s)
    {
        char* end;
        value = strtod(s.c_str(), &end);
    }
    double value;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);
