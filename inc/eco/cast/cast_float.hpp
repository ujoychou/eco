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
class float_to_string : public eco::cast::string_result
{
public:
    inline bool operator()(double v, int precision, bool_t percent)
	{
        (void)v;
        (void)precision;
        (void)percent;
    }
};


////////////////////////////////////////////////////////////////////////////////
class string_to_float
{
public:
    inline string_to_float(const eco::string_view& s)
    {
        value = atof(s.c_str());
    }
    float value;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);
