#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-08-22] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <stdint.h>
#include <string>
#include <eco/string_view.hpp>


eco_namespace(eco)
////////////////////////////////////////////////////////////////////////////////
template<typename type_t, int id>
class decoration
{
public:
	inline decoration(const type_t& v) : value(v) {}
	const type_t& value;
};
template<typename type_t> using square_t = decoration<type_t, 1>;
template<typename type_t> using group_t = decoration<type_t, 2>;
template<typename type_t> using brace_t = decoration<type_t, 3>;
template<typename type_t> using space_t = decoration<type_t, 4>;
template<typename type_t> using upper_t = decoration<type_t, 5>;
template<typename type_t> using lower_t = decoration<type_t, 6>;

template<typename type_t>
inline square_t<type_t> square(const type_t& v) { return square_t<type_t>(v); }
template<typename type_t>
inline group_t<type_t> group(const type_t& v) { return group_t<type_t>(v); }
template<typename type_t>
inline brace_t<type_t> brace(const type_t& v) { return brace_t<type_t>(v); }
template<typename type_t>
inline space_t<type_t> space(const type_t& v) { return space_t<type_t>(v); }
template<typename type_t>
inline upper_t<type_t> upper(const type_t& v) { return upper_t<type_t>(v); }
template<typename type_t>
inline lower_t<type_t> lower(const type_t& v) { return lower_t<type_t>(v); }


////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
class stream
{
public:
	inline type_t& rthis() { return (type_t&)(*this); }

	inline type_t& operator<<(bool v)
	{
		const char* str = eco::cast(v, eco::cast::bool_format_01);
		return rthis().append(str[0], 1);
	}
	inline type_t& operator<<(char v)
	{
		return rthis().append(v, 1);
	}
	inline type_t& operator<<(int8_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(uint8_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(int16_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(uint16_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(int32_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(uint32_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(int64_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(uint64_t v)
	{
		eco::c_str str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(float v)
	{
		eco::c_str str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(double v)
	{
		eco::c_str str(v);
		return rthis().append(str.c_str(), str.size());
	}
	inline type_t& operator<<(const char* v)
	{
		return rthis().append(v, (uint32_t)strlen(v));
	}
	inline type_t& operator<<(const eco::string_view& v)
	{
		return rthis().append(v.c_str(), v.size());
	}
	inline type_t& operator<<(const eco::c_str& v)
	{
		return rthis().append(v.c_str(), v.size());
	}
	inline type_t& operator<<(const std::string& v)
	{
		return rthis().append(v.c_str(), v.size());
	}

public:
	template<typename type_t>
	inline type_t& operator<<(const group_t<type_t>& v)
	{
		return rthis() << '(' << v.value << ')';
	}
	template<typename type_t>
	inline type_t& operator<<(const square_t<type_t>& v)
	{
		return rthis() << '[' << v.value << ']';
	}
	template<typename type_t>
	inline type_t& operator<<(const brace_t<type_t>& v)
	{
		return rthis() << '{' << v.value << '}';
	}
	template<typename type_t>
	inline type_t& operator<<(const space_t<type_t>& v)
	{
		return rthis() << ' ' << v.value;
	}
	template<typename type_t>
	inline type_t& operator<<(const upper_t<type_t>& v)
	{
		return rthis() << to_upper(v.value);
	}
	template<typename type_t>
	inline type_t& operator<<(const lower_t<type_t>& v)
	{
		return rthis() << to_lower(v.value);
	}
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco)