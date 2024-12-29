#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-11-14] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/cast.hpp>
#include <eco/string/string_c.hpp>
#include <string>


eco_namespace(eco)
eco_namespace(detail)
struct format_data
{
	const char* format;
	eco::string_c args;
};

eco_namespace_end(detail)
////////////////////////////////////////////////////////////////////////////////
template<typename actual_t>
class format
{
public:
	inline format(const char* f = NULL) : data(this_format())
	{
	}

	template<typename type_t>
	inline actual_t& arg(type_t value)
	{
		return rthis();
	}

	inline actual_t& arg(double value, uint32_t precision, bool_t percent)
	{
		//eco::c_str s(value, precision, percent);
		//args.append(s.value(), s.size());
		return rthis();
	}

	template<typename type_t>
	inline actual_t& operator % (type_t value)
	{
		//eco::c_str cs(value)
		//return this->data->append(cs.value(), cs.size());
		return rthis();
	}

	inline void result()
	{

	}

protected:
	inline actual_t& rthis() { return (actual_t&)(*this); }

	static eco::detail::format_data& this_format();

	eco::detail::format_data& data;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco)