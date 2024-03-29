#ifndef ECO_CAST_H
#define ECO_CAST_H
/*******************************************************************************
@ name
convert types.

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-01-15.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Prec.h>
#include <string>
#include <cstring>
#include <algorithm>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
enum Scale
{
	dec,
	hex, 
};


////////////////////////////////////////////////////////////////////////////////
template<typename int_t>
class Integer
{
public:
	inline Integer() {}

	inline explicit Integer(
		int_t v, Scale s = eco::dec,
		uint8_t width = 0, char hold = '0')
	{
		set_value(v, s, width, hold);
	}

	inline void set_value(
		int_t v, Scale s = eco::dec,
		uint8_t width = 0, char hold = '0')
	{
		if (s == dec)
		{
			m_size = convert_dec(m_buf, v, width, hold);
		}
		else if (s == hex)
		{
			m_buf[0] = '0';
			m_buf[1] = 'x';
			m_size = convert_hex(m_buf + 2, v, width, hold);
			m_size += 2;
		}
	}

	inline static uint32_t convert_dec(
		char buf[],	int_t v,
		uint8_t width = 0, char hold = '0')
	{
		char* p = buf;
		int_t x = v;

		do
		{
			int lsd = static_cast<int>(x % 10);
			x /= 10;
			*p++ = s_zero[lsd];
		} while (x != 0);

		// positive
		if (v < 0)
		{
			*p++ = '-';
		}

		// placeholder.
		if (width > 0)
		{
			int hold_size = width - (uint8_t)(p - buf);
			while (hold_size-- > 0)
			{
				*p++ = hold;
			}
		}

		*p = '\0';
		std::reverse(buf, p);
		return static_cast<uint32_t>(p - buf);
	}

	inline static uint32_t convert_hex(
		char buf[],
		int_t v,
		uint8_t width = 0,
		char hold = '0')
	{
		char* p = buf;

		do
		{
			int lsd = static_cast<int>(v % 16);
			v /= 16;
			*p++ = s_digits_hex[lsd];
		} while (v != 0);

		// placeholder.
		if (width > 0)
		{
			uint8_t hold_size = width - (uint8_t)(p - buf);
			while (hold_size--)
			{
				*p++ = hold;
			}
		}

		*p = '\0';
		std::reverse(buf, p);
		return static_cast<uint32_t>(p - buf);
	}

	inline uint32_t size() const { return m_size; }
	inline const char* c_str() const { return m_buf; }
	
private:
	char m_buf[32];
	uint32_t m_size;
	static const char s_digits[];
	static const char s_digits_hex[];
	static const char* s_zero;
};
template<typename int_t>
const char Integer<int_t>::s_digits[] = "9876543210123456789";
template<typename int_t>
const char Integer<int_t>::s_digits_hex[] = "0123456789ABCDEF";
template<typename int_t>
const char* Integer<int_t>::s_zero = s_digits + 9;


////////////////////////////////////////////////////////////////////////////////
class Double
{
public:
	inline Double(double v, int precision = -1, bool percent = false)
	{
		m_size = (uint32_t)convert(m_buf, v, precision, percent);
	}

	static int convert(char buf[], double v, int precision, bool percent)
	{
		if (percent) v *= 100;

		int size = (precision > -1)
			? snprintf(buf, 64, "%.*f", precision, v)
			: snprintf(buf, 64, "%f", v);
		if (size == -1)		// convert fail, set buf empty.
		{
			size = 0;
			buf[size] = '\0';
		}

		if (percent)
		{
			buf[size++] = '%';
			buf[size++] = '\0';
		}
		return size;
	}

	inline uint32_t size() const { return m_size; }
	inline const char* c_str() const { return m_buf; }

private:
	char m_buf[64];
	uint32_t m_size;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(detail);
inline bool is_lower(IN char v)
{
	return v >= 'a' && v <= 'z';
}
inline char upper(IN char v)
{
	return is_lower(v) ? (v + 'A' - 'a') : v;
}
inline bool iequal(IN const char* s1, IN const char* s2)
{
	char c1 = 0, c2 = 0;
	for (size_t i = 0; true; ++i)
	{
		c1 = upper(s1[i]);
		c2 = upper(s2[i]);
		if (c1 != c2 || c1 == 0 || c2 == 0) break;
	}
	return c1 == 0 && c2 == 0;
}
inline char first(IN const char* sv)
{
	return (sv != nullptr && sv[0] != '\0') ? sv[0] : 0;
}
ECO_NS_END(detail);


////////////////////////////////////////////////////////////////////////////////
inline int64_t atoi64(IN const char* sv)
{
#ifdef ECO_WIN
	return _atoi64(sv);
#else
	char* end = nullptr;
	return strtoll(sv, &end, 10);
#endif
}
inline uint64_t atoui64(IN const char* sv)
{
#ifdef ECO_WIN
	char* end = nullptr;
	return _strtoui64(sv, &end, 10);
#else
	char* end = nullptr;
	return strtoull(sv, &end, 10);
#endif
}
inline void cast(OUT bool& v, IN const char* sv)
{
	v = (sv != nullptr) && (sv[0] == '1' || eco::detail::iequal(sv, "true"));
}
inline void cast(OUT char& v, IN const char* sv)
{
	v = eco::detail::first(sv);
}
inline void cast(OUT unsigned char& v, IN const char* sv)
{
	v = eco::detail::first(sv);
}
inline void cast(OUT int16_t& v, IN const char* sv)
{
	v = static_cast<int16_t>(atoi(sv));
}
inline void cast(OUT uint16_t& v, IN const char* sv)
{
	v = static_cast<uint16_t>(atoi(sv));
}
inline void cast(OUT int32_t& v, IN const char* sv)
{
	v = atoi(sv);
}
inline void cast(OUT uint32_t& v, IN const char* sv)
{
	v = static_cast<uint32_t>(eco::atoui64(sv));
}
inline void cast(OUT int64_t& v, IN const char* sv)
{
	v = eco::atoi64(sv);
}
inline void cast(OUT uint64_t& v, IN const char* sv)
{
	v = eco::atoui64(sv);
}
inline void cast(OUT double& v, IN const char* sv)
{
	char* end = NULL;
	v = strtod(sv, &end);
	if (end != NULL && *end == '%')		// handle '%'.
		v /= 100;
}
inline void cast(OUT float& v, IN const char* sv)
{
	char* end = NULL;
	v = static_cast<float>(strtod(sv, &end));
	if (end != NULL && *end == '%')		// handle '%'.
		v /= 100;
}
template<typename dest_t>
inline dest_t cast(IN const char* v)
{
	dest_t dv;
	cast(dv, v);
	return dv;
}
template<typename dest_t>
inline dest_t cast(IN const std::string& v)
{
	return cast<dest_t>(v.c_str());
}


////////////////////////////////////////////////////////////////////////////////
/*using scene:
1.sv=20150894; cast(sv, 4) = 2015;
*/
template<uint32_t size> struct get_char
{
	inline get_char(IN const char* sv, IN uint32_t len)
	{
		str[0] = 0;
		if (len == 0) { return; }
		if (len > size - 1) { len = size - 1; }
		memcpy(str, sv, len);
		str[len] = 0;
	}
	inline operator const char*() const { return str; }
	inline const char* c_str() const { return str; }
	char str[size];
};
inline void cast(OUT double& v, IN const char* sv, IN uint32_t len)
{
	char* end = NULL;
	v = strtod(get_char<32>(sv, len), &end);
	// handle '%'.
	if (end != NULL && *end == '%') { v /= 100; }
}
inline void cast(OUT float& v, IN const char* sv, IN uint32_t len)
{
	char* end = NULL;
	v = static_cast<float>(strtod(get_char<32>(sv, len), &end));
	// handle '%'.
	if (end != NULL && *end == '%') { v /= 100; }
}
inline void cast(OUT uint64_t& v, IN const char* sv, IN uint32_t len)
{
#ifdef ECO_WIN
	char* end = nullptr;
	v = _strtoui64(get_char<24>(sv, len), &end, 10);
#else
	char* end = nullptr;
	v = strtoull(get_char<24>(sv, len).str, &end, 10);
#endif
}
inline void cast(OUT int64_t& v, IN const char* sv, IN uint32_t len)
{
#ifdef ECO_WIN
	v = _atoi64(get_char<24>(sv, len));
#else
	char* end = nullptr;
	v = strtoll(get_char<24>(sv, len).str, &end, 10);
#endif
}
inline void cast(OUT int16_t& v, IN const char* sv, IN uint32_t len)
{
	v = static_cast<int16_t>(atoi(get_char<8>(sv, len)));
}
inline void cast(OUT uint16_t& v, IN const char* sv, IN uint32_t len)
{
	v = static_cast<uint16_t>(atoi(get_char<8>(sv, len)));
}
inline void cast(OUT int32_t& v, IN const char* sv, IN uint32_t len)
{
	v = atoi(get_char<16>(sv, len));
}
inline void cast(OUT uint32_t& v, IN const char* sv, IN uint32_t len)
{
	v = atoi(get_char<16>(sv, len));
}
template<typename dest_t>
inline dest_t cast(IN const char* v, IN uint32_t siz)
{
	dest_t dv;
	cast(dv, v, siz);	// using get_char<>;
	return dv;
}
template<typename dest_t>
inline dest_t cast(IN const std::string& v, IN uint32_t siz)
{
	if (siz > v.size()) siz = v.size();
	return cast<dest_t>(v.c_str(), siz);
}


////////////////////////////////////////////////////////////////////////////////
inline std::string cast(IN bool v)
{
	return v ? "1" : "0";
}
inline std::string cast(IN int16_t v)
{
	return Integer<int16_t>(v, dec).c_str();
}
inline std::string cast(IN uint16_t v)
{
	return Integer<uint16_t>(v, dec).c_str();
}
inline std::string cast(IN int32_t v)
{
	return Integer<int32_t>(v, dec).c_str();
}
inline std::string cast(IN uint32_t v)
{
	return Integer<uint32_t>(v, dec).c_str();
}
inline std::string cast(IN int64_t v)
{
	return Integer<int64_t>(v, dec).c_str();
}
inline std::string cast(IN uint64_t v)
{
	return Integer<uint64_t>(v, dec).c_str();
}
inline std::string cast(IN double v)
{
	return Double(v).c_str();
}
inline std::string cast(IN double v, IN int precision, IN bool percent = false)
{
	return Double(v, precision, percent).c_str();
}
template<typename dest_t>
inline dest_t cast(IN double v, IN int precision, IN bool percent = false)
{
	return Double(v, precision, percent).c_str();
}


////////////////////////////////////////////////////////////////////////////////
inline char to_upper(IN char c)
{
	if (c <= 'z' && c >= 'a')
		return c + ('A' - 'a');
	return c;
}
inline char to_lower(IN char c)
{
	if (c <= 'Z' && c >= 'A')
		return c + ('a' - 'A');
	return c;
}
inline void to_upper(IN std::string& v)
{
	for (char* ch = &v[0]; *ch != 0; ++ch)
	{
		*ch = toupper(*ch);
	}
}
inline void to_lower(IN std::string& v)
{
	for (char* ch = &v[0]; *ch != 0; ++ch)
	{
		*ch = to_lower(*ch);
	}
}
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif