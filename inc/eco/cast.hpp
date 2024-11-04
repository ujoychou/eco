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
#include <eco/cast/cast_float.hpp>	
#include <eco/cast/cast_double.hpp>
#include <eco/cast/cast_integer.hpp>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
inline bool cast(const eco::string_view& v, eco::bool_format format)
{
	if (v.null()) { return false; }
	if (format == eco::bool_format_01)
		return v[0] == '1';
	if (format == eco::bool_format_tf)
		return iequal(v.c_str(), "true", v.size());
	if (format == eco::bool_format_yn)
		return iequal(v.c_str(), "yes", v.size());
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// cast string to type
template <typename T>
inline T cast(const eco::string_view& v);

template<> inline bool cast(const eco::string_view& v)
{
	return cast(v, bool_format_01);
}
template<> inline int8_t cast(const eco::string_view& v)
{
	return static_cast<uint8_t>(string_to_integer<int32_t>(v).value);
}
template<> inline uint8_t cast(const eco::string_view& v)
{
	return static_cast<uint8_t>(string_to_integer<int32_t>(v).value);
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
template<> inline float cast(const eco::string_view& v)
{
	return string_to_float(v).value;
}
template<> inline double cast(const eco::string_view& v)
{
	return string_to_double(v).value;
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
		static_cast<float_to_string&>(result)(v, precision, percent);
	}

	c_str(double v, uint32_t precision, bool_t percent)
	{
		static_cast<double_to_string&>(result)(v, precision, percent);
	}
	
private:
	eco::cast_detail::string_result result;
};


////////////////////////////////////////////////////////////////////////////////
// cast type to string
inline const char* cast(
	bool v, eco::bool_format format = eco::bool_format_01)
{
	if (format == eco::bool_format_01)
		return v ? "1" : "0";
	if (format == eco::bool_format_tf)
		return v ? "true" : "false";
	if (format == eco::bool_format_yn)
		return v ? "yes" : "no";
	return "bool";
}
inline std::string cast(
	int16_t v, uint32_t base = 0, uint32_t width = 0, char hold = ' ')
{
	eco::c_str s(v, base, width, hold);
	return std::string(s, s.size());
}
inline std::string cast(
	uint16_t v, uint32_t base = 0, uint32_t width = 0, char hold = ' ')
{
	eco::c_str s(v, base, width, hold);
	return std::string(s, s.size());
}
inline std::string cast(
	int32_t v, uint32_t base = 0, uint32_t width = 0, char hold = ' ')
{
	eco::c_str s(v, base, width, hold);
	return std::string(s, s.size());
}
inline std::string cast(
	uint32_t v, uint32_t base = 0, uint32_t width = 0, char hold = ' ')
{
	eco::c_str s(v, base, width, hold);
	return std::string(s, s.size());
}
inline std::string cast(
	int64_t v, uint32_t base = 0, uint32_t width = 0, char hold = ' ')
{
	eco::c_str s(v, base, width, hold);
	return std::string(s, s.size());
}
inline std::string cast(
	uint64_t v, uint32_t base = 0, uint32_t width = 0, char hold = ' ')
{
	eco::c_str s(v, base, width, hold);
	return std::string(s, s.size());
}
inline std::string cast(double v)
{
	eco::c_str s(v, 0, false);
	return std::string(s, s.size());
}
inline std::string cast(double v, int precision, bool_t percent = false)
{
	eco::c_str s(v, precision, percent);
	return std::string(s, s.size());
}

////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);
