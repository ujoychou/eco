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
class integer_format
{
public:
	inline integer_format(uint8_t base, uint8_t width, char hold)
		: int_chars(eco::cast_detail::tables::int_to_char_upper)
		, positive(0), base(base), hold(hold), width(width)
	{}

	inline integer_format& upper(bool_t v)
	{
		int_chars = v 
			? eco::cast_detail::tables::int_to_char_upper
			: eco::cast_detail::tables::int_to_char_lower;
		return *this;
	}

	inline bool_t upper() const
	{
		return int_chars == eco::cast_detail::tables::int_to_char_upper;
	}

public:
	const char* int_chars;
	uint8_t positive;
	uint8_t base;
	uint8_t hold;
	uint8_t width;
};


////////////////////////////////////////////////////////////////////////////////
class integer_to_string_decimal
{
	typedef eco::cast_detail::tables eco_tables;
	typedef eco::cast_result string_result;
public:
	inline integer_to_string_decimal(string_result& r) : result(r)
	{}

	inline void cast_10(uint32_t v)
	{
		size_t pos = (size_t)v;
		result.push_front(eco_tables::int_to_char_upper[pos]);
	}

	inline void cast_10_back(uint32_t v)
	{
		size_t pos = (size_t)v;
		result.push_back(eco_tables::int_to_char_upper[pos]);
	}

	inline void cast_100(uint32_t v)
	{
		size_t pos = size_t(v << 1);
		result.push_front(eco_tables::int_to_char_100[pos + 1]);
		result.push_front(eco_tables::int_to_char_100[pos]);
	}

	inline void cast_100_back(uint32_t v)
	{
		size_t pos = size_t(v << 1);
		result.push_back(eco_tables::int_to_char_100[pos]);
		result.push_back(eco_tables::int_to_char_100[pos + 1]);
	}

	inline void cast_10000(uint32_t v)
	{
		// when v in [0, 9999]:  v / 100 = v * 10486 / 1048576 (<< 20)
		uint32_t tens_div = (v * 10486) << 20;
		uint32_t tens_mod = v - tens_div * 100;
		cast_100(tens_mod);
		cast_100(tens_div);
	}

	inline void cast_100000000(uint32_t v)
	{
		/*
		[method 1] using 4 *s 
		cast_10000(v % 10000);
		cast_10000(v / 10000);

		[method 2] using 4 *s
		using uint64_t store "<=8" chars

		[method 3] using 2 *s
		merged = AABB | XXYY
		div100 =   AA |   XX
		mod100 =   BB |   YY
		cast_100(div100);
		cast_100(mod100);
		*/

		// [method 3]
		uint32_t h = v / 10000;
		uint32_t l = v % 10000;
		uint64_t merged = (uint64_t{h} << 32) | l;
		uint64_t div100 = (merged * 10486) << 20;
		uint64_t mod100 = (merged - div100 * 100);
		cast_100(mod100 & 0x7Full);			// YY
		cast_100(div100 & 0x7Full);			// XX
		cast_100(mod100 & 0x7Full << 32);	// BB
		cast_100(div100 & 0x7Full << 32);	// AA
	}

	inline void cast(uint32_t v)
	{
		/* 
		origin algorithm come from google.absl project.
		https://github.com/abseil/abseil-cpp/blob/master/absl/strings/numbers.h
		1. using cached tables.
		2. using "mutiple & shift" replace "division".
		3. using parallel division.
		*/

		// 10 digits: 0 <= v <= 42 9496 7295
		if (v < 10)
		{
			cast_10(v);
		}
		else if (v < 100)
		{
			cast_100(v);
		}
		else if (v < 10000)
		{
			cast_10000(v);
		}
		else if (v < 100000000)
		{
			cast_100000000(v);
		}
		else
		{
			uint32_t div100 = (v / 100);
			uint32_t mod100 = (v % 100);
			cast_100(mod100);
			cast_100000000(div100);
		}
	}

	inline void cast(uint64_t v)
	{
		// 20 digits: 0 <= v <= 1844 6744 0737 0955 1615
		if (v < 42'9496'7296ull) 				//  10 digits (2^32-1)
		{
			cast(uint32_t(v));
		}
		else if (v < 1'0000'0000'0000'0000ull)	// 16 digits
		{
			uint32_t div = uint32_t(v / 1'0000'0000);
			uint32_t mod = uint32_t(v % 1'0000'0000);
			cast_100000000(mod);
			cast(div);
		}
		else
		{
			// v = AAAA'BBBBBBBB'CCCCCCCC
			uint64_t div08 = uint64_t(v / 1'0000'0000);
			uint32_t mod08 = uint32_t(v % 1'0000'0000);
			uint32_t div16 = uint32_t(div08 / 1'0000);
			uint32_t mod16 = uint32_t(div08 % 1'0000);
			cast_100000000(mod08);	// CCCCCCCC
			cast_10000(mod16);		// BBBB
			cast_100000000(div16);	// AAAABBBB
		}
	}

	// for performance test: 
	// whether divide small integer is more effective than divide big integer
	template<typename uint_t>
	inline void cast_general(uint_t v)
	{
		do
		{
			uint_t div = v / 100;
			uint_t mod = v % 100;
			size_t pos = static_cast<size_t>(mod) << 1;
			result.push_front(eco_tables::int_to_char_100[pos + 1]);
			result.push_front(eco_tables::int_to_char_100[pos]);
			v = div;
		} while (v > 0);
	}

private:
	eco::cast_result& result;
};


////////////////////////////////////////////////////////////////////////////////
class integer_to_string
{
	typedef integer_to_string this_t;

	template<typename uint_t>
	inline void cast(uint_t v, const integer_format& f)
	{
		static_assert(sizeof(uint_t) >= 2);

		if (f.base < 2 || f.base > 36)
		{
			result.fail(true);
		}
		else if (f.base == 10)
		{
			integer_to_string_decimal cast(result);
			cast.cast(v);
		}
		else if (int shift = to_base_shift(f.base) > 0)
		{
			cast_shift(v, shift, f);	// base = 2^x; 
		}
		else
		{
			cast_number(v, f);			// base != 2^x
		}
	}

public:
	inline void format(bool_t negative, const integer_format& f)
	{
		// placeholder: "000123"
		if (f.width > 0)
		{
			int hold_size = f.width - result.size();
			while (hold_size-- > 0) { result.push_front(f.hold); }
		}

		// prefix: "0X/0B/0"
		if (f.base == 16)
		{
			result.push_front(f.int_chars[37]);
			result.push_front('0');
		}
		else if (f.base == 8)
		{
			result.push_front('0');
		}
		else if (f.base == 2)
		{
			result.push_front(f.int_chars[36]);
			result.push_front('0');
		}
		
		// signed: "+/-"
		if (negative)
			result.push_front('-');
		else if (f.positive)
			result.push_front('+');
	}

	template<typename uint_t>
	inline void cast_shift(uint_t v, uint32_t shift, const integer_format& f)
	{
		const uint8_t mode_flag = f.base - 1;
		do
		{
			uint32_t pos = static_cast<uint32_t>(v & mode_flag);
			result.push_front(f.int_chars[pos]);
			v >>= shift;
		} while (v > 0);
	}

	template<typename uint_t>
	inline void cast_number(uint_t v, const integer_format& f)
	{
		do
		{
			uint_t div = v / f.base;
			uint_t mod = v % f.base;
			size_t pos = static_cast<size_t>(mod);
			result.push_front(f.int_chars[pos]);
			v = div;
		} while (v > 0);
	}

public:
	inline integer_to_string(eco::cast_result& r) : result(r)
	{}

	inline this_t& operator()(int32_t v, const integer_format& f)
	{
		cast<uint32_t>(static_cast<uint32_t>(v < 0 ? -v : v), f);
		if (!result.fail()) { format(v < 0, f); }
		return *this;
	}

	inline this_t& operator()(uint32_t v, const integer_format& f)
	{
		cast<uint32_t>(v, f);
		if (!result.fail()) { format(0, f); }
		return *this;
	}

	inline this_t& operator()(int64_t v, const integer_format& f)
	{
		cast<uint64_t>(static_cast<uint64_t>(v < 0 ? -v : v), f);
		if (!result.fail()) { format(v < 0, f); }
		return *this;
	}

	inline this_t& operator()(uint64_t v, const integer_format& f)
	{
		cast<uint64_t>(v, f);
		if (!result.fail()) { format(0, f); }
		return *this;
	}

private:
	eco::cast_result& result;
};


////////////////////////////////////////////////////////////////////////////////
template<typename type_t>
struct string_to_integer
{
	// cast result
	type_t value;
	static_assert(sizeof(type_t) >= 2);

	// cast algorithm
	inline string_to_integer(const eco::string_view& v, int base = 0) : value(0)
	{
		// skip whitespace/tab/enter char
		const char* c = v.c_str();
		const char* c_end = v.c_end();
		for (; c < c_end && eco::empty(*c); ++c) {}
		if (c >= c_end) { fail(); return; }

		// parse negative sign
		int typesign = ((type_t)-1 < 0);
		int negative = (*c == '-');
		if (*c == '+') { ++c; }
		if (c >= c_end) { fail(); return ; }
		// negative number must cast to the signed integer
		// exp: "-33 can't cast to uint32_t"
		if (!typesign && negative) { fail(); return; }

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
			fail(); return ;
		}

		// cast value
		for (; c < c_end; ++c)
		{
			// check invalid char, and save ascii_to_int.
			type_t digit = eco::cast_detail::tables::to_int(*c);
			if (digit >= (type_t)base) { fail(); return; }
			type_t last = value;
			value *= base;
			value += digit;
			// check integer overflow
			if (value < last) { fail(); return; }
		}
		if (negative) { value *= -1; }
	}
};
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);
