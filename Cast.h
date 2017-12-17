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
#include <cstdint>
#include <string>
#include <eco/Export.h>
#include <eco/Memory.h>


namespace eco{;
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
	explicit Integer(
		int_t v, Scale s = eco::dec,
		uint8_t width = 0, char hold = '0')
	{
		set_value(v, s, width, hold);
	}

	void set_value(
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

	static uint32_t convert_dec(
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

	static uint32_t convert_hex(
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


public:
	inline operator const char*() const
	{
		return m_buf;
	}

	inline const char* c_str() const
	{
		return m_buf;
	}

	inline uint32_t size() const
	{
		return m_size;
	}

private:
	char m_buf[64];
	uint32_t m_size;

private:
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
	Double(double v, int precision = -1)
	{
		m_size = convert(m_buf, v, precision, false);
	}

	Double(double v, int precision, bool percent)
	{
		m_size = convert(m_buf, v, precision, percent);
	}

	static int convert(char buf[], double v, int precision, bool percent)
	{
		if (percent) {
			v *= 100;
		}

		int size = 0;
		if (precision > -1) {
			size = snprintf(buf, 64, "%.*f", precision, v);
		}
		else {
			size = snprintf(buf, 64, "%f", v);
		}

		if (percent)
		{
			buf[size++] = '%';
			buf[size++] = '\0';
		}
		return size;
	}

public:
	inline operator const char*() const
	{
		return m_buf;
	}

	inline const char* c_str() const
	{
		return m_buf;
	}

	inline int size() const
	{
		return m_size;
	}

private:
	char m_buf[64];
	int m_size;
};

////////////////////////////////////////////////////////////////////////////////
inline void cast(OUT bool& v, IN const char* sv)
{
	v = (sv != nullptr && sv[0] == '1');
}
inline void cast(OUT char& v, IN const char* sv)
{
	v = (sv != nullptr && sv[0] != '\0') ? sv[0] : 0;
}
inline void cast(OUT unsigned char& v, IN const char* sv)
{
	v = (sv != nullptr && sv[0] != '\0') ? sv[0] : 0;
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
	v = atoi(sv);
}
inline void cast(OUT int64_t& v, IN const char* sv)
{
#ifdef ECO_WIN
	v = _atoi64(sv);
#else
	char* end = nullptr;
	v = strtoll(sv, &end, 10);
#endif
}
inline void cast(OUT uint64_t& v, IN const char* sv)
{
#ifdef ECO_WIN
	char* end = nullptr;
	v = _strtoui64(sv, &end, 10);
#else
	char* end = nullptr;
	v = strtoull(sv, &end, 10);
#endif
}
inline void cast(OUT double& v, IN const char* sv)
{
	char* end = NULL;
	v = strtod(sv, &end);

	// handle '%'.
	if (end != NULL && *end == '%'){
		v /= 100;
	}
}


////////////////////////////////////////////////////////////////////////////////
// set a char musk to string pos.
class MaskChar
{
public:
	inline MaskChar(IN const char* sv, IN const uint32_t pos)
	{
		m_str = &sv[pos];
		m_val = *m_str;
		(char&)(*m_str) = '\0';
	}
	inline ~MaskChar()
	{
		(char&)(*m_str) = m_val;
	}

	char m_val;
	const char* m_str;
};


/*using scene:
1.sv=20150894; cast(sv, 4) = 2015;
*/
////////////////////////////////////////////////////////////////////////////////
inline void cast(OUT double& v, IN const char* sv, IN const uint32_t len)
{
	MaskChar mask(sv, len);
	char* end = NULL;
	v = strtod(sv, &end);
	// handle '%'.
	if (end != NULL && *end == '%'){
		v /= 100;
	}
}
inline void cast(OUT uint64_t& v, IN const char* sv, IN const uint32_t len)
{
	MaskChar mask(sv, len);
#ifdef ECO_WIN
	char* end = nullptr;
	v = _strtoui64(sv, &end, 10);
#else
	char* end = nullptr;
	v = strtoull(sv, &end, 10);
#endif
}
inline void cast(OUT int64_t& v, IN const char* sv, IN const uint32_t len)
{
	MaskChar mask(sv, len);
#ifdef ECO_WIN
	v = _atoi64(sv);
#else
	char* end = nullptr;
	v = strtoll(sv, &end, 10);
#endif
}
inline void cast(OUT int16_t& v, IN const char* sv, IN const uint32_t len)
{
	MaskChar mask(sv, len);
	v = static_cast<int16_t>(atoi(sv));
}
inline void cast(OUT uint16_t& v, IN const char* sv, IN const uint32_t len)
{
	MaskChar mask(sv, len);
	v = static_cast<uint16_t>(atoi(sv));
}
inline void cast(OUT int32_t& v, IN const char* sv, IN const uint32_t len)
{
	MaskChar mask(sv, len);
	v = atoi(sv);
}
inline void cast(OUT uint32_t& v, IN const char* sv, IN const uint32_t len)
{
	MaskChar mask(sv, len);
	v = atoi(sv);
}




////////////////////////////////////////////////////////////////////////////////
inline void cast(OUT std::string& sv, IN bool v)
{
	sv = v ? "1" : "0";
}
inline void cast(OUT std::string& sv, IN int16_t v)
{
	sv = Integer<int16_t>(v, dec);
}
inline void cast(OUT std::string& sv, IN uint16_t v)
{
	sv = Integer<uint16_t>(v, dec);
}
inline void cast(OUT std::string& sv, IN int32_t v)
{
	sv = Integer<int32_t>(v, dec);
}
inline void cast(OUT std::string& sv, IN uint32_t v)
{
	sv = Integer<uint32_t>(v, dec);
}
inline void cast(OUT std::string& sv, IN int64_t v)
{
	sv = Integer<int64_t>(v, dec);
}
inline void cast(OUT std::string& sv, IN uint64_t v)
{
	sv = Integer<uint64_t>(v, dec);
}
inline void cast(OUT std::string& sv, IN double v)
{
	sv = Double(v);
}
inline void cast(OUT std::string& sv, IN double v,
	IN int precision, IN bool is_percent = false)
{
	sv = Double(v, precision, is_percent);
}

////////////////////////////////////////////////////////////////////////////////
template<typename dest_t, typename sour_t>
inline dest_t cast(IN const sour_t& sv)
{
	dest_t dv;
	cast(dv, sv);
	return dv;
}
template<typename dest_t>
inline dest_t cast(IN const char* v)
{
	dest_t dv;
	cast(dv, v);
	return dv;
}
template<typename dest_t>
inline dest_t cast(IN const char* v, IN const uint32_t siz)
{
	dest_t dv;
	cast(dv, v, siz);
	return dv;
}
template<typename dest_t>
inline dest_t cast(IN const std::string& v)
{
	return cast<dest_t>(v.c_str());
}
template<typename dest_t>
inline dest_t cast(IN const std::string& v, IN const uint32_t siz)
{
	return cast<dest_t>(v.c_str(), siz);
}
template<typename dest_t>
inline dest_t cast(IN double v, IN int precision, IN bool is_percent = false)
{
	dest_t dv;
	cast(dv, v, precision, is_percent);
	return dv;
}


////////////////////////////////////////////////////////////////////////////////
inline char to_upper(IN const char c)
{
	if (c <= 'z' && c >= 'a')
		return c + ('A' - 'a');
	return c;
}
inline char to_lower(IN const char c)
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
}
#endif