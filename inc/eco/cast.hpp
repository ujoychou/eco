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
#include <eco/cast/cast_double.hpp>
#include <eco/cast/cast_integer.hpp>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
// cast string to type
template <typename T>
inline T cast(const eco::string_view& v);

template <typename T>
inline T cast(const eco::string_view& v, uint32_t size)
{
	return cast<T>(v.size(size));
}

template<> inline bool cast(const eco::string_view& v)
{
	return !v.null() && (v[0] == '1' || eco::iequal(v.c_str(), "true"));
}
template<> inline int8_t cast(const eco::string_view& v)
{
	return v[0];
}
template<> inline uint8_t cast(const eco::string_view& v)
{
	return v[0];
}
template<> inline int16_t cast(const eco::string_view& v)
{
	return string_to_integer<int16_t>(v).value;
}
template<> inline uint16_t cast(const eco::string_view& v)
{
	return string_to_integer<uint16_t>(v).value;
}
template<> inline int32_t cast(const eco::string_view& v)
{
	return string_to_integer<int32_t>(v).value;
}
template<> inline uint32_t cast(const eco::string_view& v)
{
	return string_to_integer<uint32_t>(v).value;
}
template<> inline int64_t cast(const eco::string_view& v)
{
	return string_to_integer<int64_t>(v).value;
}
template<> inline uint64_t cast(const eco::string_view& v)
{
	return string_to_integer<uint64_t>(v).value;
}


////////////////////////////////////////////////////////////////////////////////
// cast type to const c_str(char*)
class c_str
{
public:
	inline operator const char*() const
	{
		return result.c_str();
	}

	inline const char* value() const
	{
		return result.c_str();
	}

	inline uint32_t size() const
	{
		return result.size();
	}

	template<typename integer_t> inline
	c_str(integer_t v, uint32_t base = 0, uint32_t width = 0, char hold = ' ')
	{
		integer_to_string_format fmt(base, width, hold);
		static_cast<integer_to_string<integer_t>&>(result)(v, fmt);
	}

	c_str(float v, uint32_t precision, bool_t percent)
	{
		//double_to_string_format fmt(base, width, hold);
		//static_cast<double_to_string<integer_t>&>(result)(v, fmt);
	}

	c_str(double v, uint32_t precision, bool_t percent)
	{
		//float_to_string_format fmt(base, width, hold);
		//static_cast<double_to_string<integer_t>&>(result)(v, fmt);
	}

private:
	eco::cast::string_result result;
};


////////////////////////////////////////////////////////////////////////////////
// cast type to string
inline std::string cast(bool v, cast::bool_format format = cast::bool_format_01)
{
	if (format == cast::bool_format_01)
		return v ? "1" : "0";
	if (format == cast::bool_format_tf)
		return v ? "true" : "false";
	if (format == cast::bool_format_yn)
		return v ? "yes" : "no";
	return "bool";
}
inline std::string cast(int16_t v, uint32_t base = 0, uint32_t width = 0, char hold = ' ')
{
	return eco::c_str(v, base, width, hold).value();
}
inline std::string cast(uint16_t v, uint32_t base = 0, uint32_t width = 0, char hold = ' ')
{
	return eco::c_str(v, base, width, hold).value();
}
inline std::string cast(int32_t v, uint32_t base = 0, uint32_t width = 0, char hold = ' ')
{
	return eco::c_str(v, base, width, hold).value();
}
inline std::string cast(uint32_t v, uint32_t base = 0, uint32_t width = 0, char hold = ' ')
{
	return eco::c_str(v, base, width, hold).value();
}
inline std::string cast(int64_t v, uint32_t base = 0, uint32_t width = 0, char hold = ' ')
{
	return eco::c_str(v, base, width, hold).value();
}
inline std::string cast(uint64_t v, uint32_t base = 0, uint32_t width = 0, char hold = ' ')
{
	return eco::c_str(v, base, width, hold).value();
}
inline std::string cast(double v)
{
	return std::string(c_str(v, 0, false));
}
inline std::string cast(double v, int precision, bool_t percent = false)
{
	return std::string(c_str(v, precision, percent));
}

////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);
