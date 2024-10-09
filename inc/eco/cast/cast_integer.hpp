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
#include <eco/cast/cast.hpp>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
class integer_to_string_format
{
public:
	inline integer_to_string_format& upper(bool_t v)
	{
		if (v)
		{
			int_chars = eco::cast::tables::int_to_char_upper;
			prefix_bin = 'B';
			prefix_hex = 'X';
		}
		else
		{
			int_chars = eco::cast::tables::int_to_char_lower;
			prefix_bin = 'b';
			prefix_hex = 'x';
		}
		return *this;
	}

	inline bool_t has_positive() const
	{
		return positive != 0;
	}

	inline integer_to_string_format()
		: prefix_bin('B'), prefix_hex('X'), negative('-'), positive(0)
		, base(10), fast_dec(0), placehold(' '), placehold_width(0)
		, int_chars(eco::cast::tables::int_to_char_upper)
	{}

	inline integer_to_string_format(
		uint8_t base = 10, uint32_t width = 0, char hold = ' ')
		: prefix_bin('B'), prefix_hex('X'), negative('-'), positive(0)
		, base(base), fast_dec(0), placehold(hold), placehold_width(width)
		, int_chars(eco::cast::tables::int_to_char_upper)
	{}

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
template<typename type_t>
class integer_to_string : public eco::cast::string_result
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
	inline void set_prefix(bool_t negative, const integer_to_string_format& fmt)
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
				buff_[--size_] = eco::cast::tables::int_to_char_upper[pos];
				break;
			}
			else if (positive < 100)
			{
				size_t pos = size_t(positive << 1);
				buff_[--size_] = eco::cast::tables::int_to_char_100[pos + 1];
				buff_[--size_] = eco::cast::tables::int_to_char_100[pos];
				break;
			}
			else
			{
				do
				{
					size_t pos = size_t(positive % 100) << 1;
					buff_[--size_] = eco::cast::tables::int_to_char_100[pos + 1];
					buff_[--size_] = eco::cast::tables::int_to_char_100[pos];
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
			type_t digit = eco::cast::tables::to_int(c);
			if (digit >= base) { return set_fail(); }
			type_t last = value;
			value *= base;
			value += digit;
			// check integer overflow
			if (value < last) { return set_fail(); }
		}
		if (negative) { value *= -1; }
	}

	bool_t fail;
	type_t value;
};
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);
