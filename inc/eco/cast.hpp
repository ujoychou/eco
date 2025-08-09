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
#include <eco/cast/cast_double.hpp>
#include <eco/cast/cast_integer.hpp>
#include <string>


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
	return static_cast<uint8_t>(eco::string_to_integer<int32_t>(v).value);
}
template<> inline uint8_t cast(const eco::string_view& v)
{
	return static_cast<uint8_t>(eco::string_to_integer<int32_t>(v).value);
}
template<> inline int16_t cast(const eco::string_view& v)
{
	return eco::string_to_integer<int16_t>(v).value;
}
template<> inline uint16_t cast(const eco::string_view& v)
{
	return eco::string_to_integer<uint16_t>(v).value;
}
template<> inline int32_t cast(const eco::string_view& v)
{
	return eco::string_to_integer<int32_t>(v).value;
}
template<> inline uint32_t cast(const eco::string_view& v)
{
	return eco::string_to_integer<uint32_t>(v).value;
}
template<> inline int64_t cast(const eco::string_view& v)
{
	return eco::string_to_integer<int64_t>(v).value;
}
template<> inline uint64_t cast(const eco::string_view& v)
{
	return eco::string_to_integer<uint64_t>(v).value;
}
template<> inline float cast(const eco::string_view& v)
{
	return eco::string_to_double<float>(v).value;
}
template<> inline double cast(const eco::string_view& v)
{
	return eco::string_to_double<double>(v).value;
}


////////////////////////////////////////////////////////////////////////////////
// cast type to const c_str(char*), shim class.
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

	inline c_str(int32_t v, uint8_t base = 0, uint8_t width = 0, char hold = ' ')
	{
		integer_to_string().format(base, width, hold).cast(v, result);
	}
	inline c_str(uint32_t v, uint8_t base = 0, uint8_t width = 0, char hold = ' ')
	{
		integer_to_string().format(base, width, hold).cast(v, result);
	}
	inline c_str(int64_t v, uint8_t base = 0, uint8_t width = 0, char hold = ' ')
	{
		integer_to_string().format(base, width, hold).cast(v, result);
	}
	inline c_str(uint64_t v, uint8_t base = 0, uint8_t width = 0, char hold = ' ')
	{
		integer_to_string().format(base, width, hold).cast(v, result);
	}

	inline c_str(float v, int prec, eco::bool_t sicentific, eco::bool_t percent)
	{
	}
	inline c_str(double v, int prec, eco::bool_t sicentific, eco::bool_t percent)
	{
	}
	
private:
	eco::cast_detail::result result;
};


////////////////////////////////////////////////////////////////////////////////
// cast bool to string
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

// cast integer to string
template<typename int_t>
inline std::string cast(int_t v, uint8_t width = 0, char hold = ' ')
{
	static_assert(std::is_integral_v<int_t> && sizeof(int_t) >= 4,
        "int_t must be a 32-bit or 64-bit integer type");
	eco::c_str s(v, 10, width, hold);
	return std::string(s, s.size());
}

// cast float to string
template<typename float_t>
inline std::string cast(float_t v, int precision = -1, bool_t percent = false)
{
	static_assert(
        std::is_floating_point_v<float_t>,
        "float_t must be a floating-point type (float or double)"
    );
	eco::c_str s(v, precision, percent);
	return std::string(s, s.size());
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);
