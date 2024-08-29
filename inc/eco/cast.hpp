#pragma once
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
#include <eco/macro.hpp>
#include <eco/string/string_c.hpp>
#include <eco/string/string_view.hpp>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
struct cast_tables
{
	// string size = 36
	static const char* int_to_char_upper;
	static const char* int_to_char_lower;
	// string size = 200
	static const char* int_to_char_100;
	static const uint8_t ascii_to_int[80];

	static inline uint8_t to_int(char c)
	{
		return (c < '0' || c > 'z') ? 36 : ascii_to_int[c - '0'];
	}
};
const char* cast_tables::int_to_char_100 = "00010203040506070809"
"101112131415161718192021222324252627282930313233343536373839"
"404142434445464748495051525354555657585960616263646566676869"
"707172737475767778798081828384858687888990919293949596979899";
const char* cast_tables::int_to_char_upper = 
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char* cast_tables::int_to_char_lower = 
"0123456789abcdefghijklmnopqrstuvwxyz";
const uint8_t cast_tables::ascii_to_int[80] = {
0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  36, 36, 36, 36, 36, 36, 36, 10, 11, 12, 
13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 
33, 34, 35, 36, 36, 36, 36, 36, 36, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 36, 36, 36, 36};


////////////////////////////////////////////////////////////////////////////////
class integer_to_string_format
{
public:
	inline integer_to_string_format& upper(Bool v)
	{
		if (v)
		{
			int_chars = cast_tables::int_to_char_upper;
			prefix_bin = 'B';
			prefix_hex = 'X';
		}
		else
		{
			int_chars = cast_tables::int_to_char_lower;
			prefix_bin = 'b';
			prefix_hex = 'x';
		}
		return *this;
	}

	inline Bool has_positive() const
	{
		return positive != 0;
	}

	inline integer_to_string_format()
		: prefix_bin('B'), prefix_hex('X'), negative('-'), positive(0)
		, base(10), fast_dec(0), placehold(' '), placehold_width(0)
		, int_chars(cast_tables::int_to_char_upper)
	{}

	inline integer_to_string_format(
		uint8_t base = 10, uint32_t width = 0, char hold = ' ')
		: prefix_bin('B'), prefix_hex('X'), negative('-'), positive(0)
		, base(base), fast_dec(0), placehold(hold), placehold_width(width)
		, int_chars(cast_tables::int_to_char_upper)
	{
		integer_to_string_format
	}

public:
	char prefix_bin;
	char prefix_hex;
	char negative;
	char positive;
	uint8_t base;
	uint8_t fast_dec;
	char placehold;
	uint32_t placehold_width;
	const char* int_chars;	
};


////////////////////////////////////////////////////////////////////////////////
struct integer_to_string_result
{
protected:
	char buff_[24];
	uint32_t size_;

	inline void init()
	{
		size_ = sizeof(buff_) - 1;
		buff_[size_] = 0;
	}

public:
	inline uint32_t size() const
	{
		return sizeof(buff_) - 1 - size_;
	}

	inline const char* c_str() const
	{
		return &buff_[size_];
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
class integer_to_string : public integer_to_string_result
{
public:
	inline bool operator()(type_t v, const integer_to_string_format& fmt)
	{
		integer_to_string_result::init();
		if (fmt.base < 2 || fmt.base > 36)
		{
			return false;
		}

		// cast dec numbers: google absl algo & using cast_table
		if (fmt.base == 10)
		{
			fmt.fast_dec ? cast_dec_fast(v, fmt) : cast_dec(v, fmt);
			return true;
		}

		// base = 2^x: cast_shift, else cast_number
		int shift = to_base_shift(fmt.base);
		(shift > 0) ? cast_shift(v, shift, fmt) : cast_number(v, fmt);
		return true;
	}

private:
	inline void set_prefix(Bool negative, const integer_to_string_format& fmt)
	{
		// placeholder: "000123"
		if (fmt.width > 0)
		{
			int hold_size = fmt.width - size();
			while (hold_size-- > 0) { buff_[--size_] = fmt.hold; }
		}

		// prefix: "0X/0B/0"
		if (fmt.base == 16)
		{
			buff_[--size_] = fmt.prefix_hex;
			buff_[--size_] = '0';
		}
		else if (fmt.base == 8)
		{
			buff_[--size_] = '0';
		}
		else if (fmt.base == 2)
		{
			buff_[--size_] = fmt.prefix_bin;
			buff_[--size_] = '0';
		}
		
		// signed: "+/-"
		if (negative)
			buff_[--size_] = fmt.negative;
		else if (fmt.has_positive)
			buff_[--size_] = fmt.positive;
	}

public:
	inline void cast_shift(
		type_t v, uint32_t shift, const integer_to_string_format& fmt)
	{
		type_t positive = to_positive(v);
		uint8_t base = fmt.base - 1;

		do
		{
			uint32_t pos = (uint32_t)(positive & base);
			buff_[--size_] = fmt.int_chars[pos];
			positive >> shift;
		} while (positive > 0);

		set_prefix(v < 0, fmt);
	}

public:
	inline void cast_dec(type_t v, const integer_to_string_format& fmt)
	{
		type_t positive = to_positive(v);
		while (true)
		{
			if (positive < 10)
			{
				size_t pos = (size_t)positive;
				buff_[--size_] = cast_tables::int_to_char_upper[pos];
				break;
			}
			else if (positive < 100)
			{
				size_t pos = size_t(positive << 1);
				buff_[--size_] = cast_tables::int_to_char_100[pos + 1];
				buff_[--size_] = cast_tables::int_to_char_100[pos];
				break;
			}
			else
			{
				do
				{
					size_t pos = size_t(positive % 100) << 1;
					buff_[--size_] = cast_tables::int_to_char_100[pos + 1];
					buff_[--size_] = cast_tables::int_to_char_100[pos];
					positive /= 100;
				} while (positive >= 100);
			}
		}
		set_prefix(v < 0, fmt);
	}

	inline void cast_dec_fast(type_t v, const integer_to_string_format& fmt)
	{
		// using "google.absl FastIntToBuffer"
	}

public:
	inline void cast_number(
		type_t v, const integer_to_string_format& fmt)
	{
		type_t positive = to_positive(v);
		do
		{ 
			size_t pos = size_t(positive % fmt.base);
			buff_[--size_] = fmt.int_chars[pos];
			positive /= fmt.base;
		} while (positive >= 100);

		set_prefix(v < 0, fmt);
	}
};

class cast_double_to_string
{
};

class cast_float_to_string
{
};

////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
struct string_to_integer
{
	inline void set_fail()
	{
		fail = 1;
	}

	inline string_to_integer(
		const eco::string_view& v, int base = 0) : value(0)
	{
		if (sizeof(type_t) < 2) { return set_fail(); }
		
		// skip whitespace/tab/enter char
		const char* c = v.c_str();
		const char* c_end = v.c_end();
		for (; c < c_end && is_empty(*c); ++c) {}
		if (c >= c_end) { return set_fail(); }

		// parse negative sign
		int typesign = ((type_t)-1 < 0);
		int negative = (*c == '-');
		if (*c == '+') { ++c; }
		if (c >= c_end) { return false; }
		// negative number must cast to the signed integer
		// exp: "-33 can't cast to uint32_t"
		if (!typesign && negative) { return set_fail(); }

		// parse integer base: 16/10/8
		if (base == 0)
		{
			base = 10;
			if (c_end - c > 2)
			{
				base = (c[0] != '0') ? 10 
					: ((c[1] == 'x' || c[1] == 'X') ? 16 : 8);
				c += (base == 8) ? 1 : (base == 16 ? 2 : 0);
			}
		}
		else if (base < 2 || base > 36) 
		{
			return set_fail();
		}

		// cast value
		for (; c < c_end; ++c)
		{
			// check invalid char, and save ascii_to_int.
			type_t digit = cast_tables::to_int(c);
			if (digit >= base) { return set_fail(); }
			type_t last = value;
			value *= base;
			value += digit;
			// check integer overflow
			if (value < last) { return set_fail(); }
		}
		if (negative) { value *= -1; }
	}

	Bool fail;
	type_t value;
};


////////////////////////////////////////////////////////////////////////////////
class cast_string_to_double
{
};

class cast_string_to_float
{
};


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
// cast type to const char* (c_str)
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

private:
	integer_to_string_result result;
};


////////////////////////////////////////////////////////////////////////////////
// cast type to string
inline std::string cast(bool v)
{
	return v ? "1" : "0";
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
	return precision(v).c_str();
}
inline std::string cast(double v, int prec, bool percent = false)
{
	return precision(v, prec, percent).c_str();
}
template<typename dest_t>
inline dest_t cast(double v, int prec, bool percent = false)
{
	return precision(v, prec, percent).c_str();
}
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);
