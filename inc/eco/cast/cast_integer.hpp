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
#include <eco/type/number.hpp>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
class integer_format
{
public:
	inline integer_format()
		: int_chars(eco::cast_detail::tables<>::CHAR_UPPER)
		, positive(0), base(10), hold(' '), width(0)
	{}

	inline integer_format(uint8_t base, uint8_t width, char hold)
		: int_chars(eco::cast_detail::tables<>::CHAR_UPPER)
		, positive(0), base(base), hold(hold), width(width)
	{}

	inline integer_format& lower(bool_t v)
	{
		int_chars = v 
			? eco::cast_detail::tables<>::CHAR_LOWER
			: eco::cast_detail::tables<>::CHAR_UPPER;
		return *this;
	}

	inline bool_t lower() const
	{
		return int_chars == eco::cast_detail::tables<>::CHAR_LOWER;
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
	typedef eco::cast_detail::result string_result;
public:
	inline integer_to_string_decimal()
	{}

	inline void cast_10(uint32_t v, string_result& r)
	{
		size_t pos = (size_t)v;
		r.push_front(eco::cast_detail::tables<>::CHAR_UPPER[pos]);
	}

	/*inline void cast_10_back(uint32_t v, string_result& r)
	{
		size_t pos = (size_t)v;
		r.push_back(eco::cast_detail::tables<>::CHAR_UPPER[pos]);
	}*/

	inline void cast_100(uint32_t v, string_result& r)
	{
		size_t pos = size_t(v << 1);
		r.push_front(eco::cast_detail::tables<>::CHAR_100[pos + 1]);
		r.push_front(eco::cast_detail::tables<>::CHAR_100[pos]);
	}

	/*inline void cast_100_back(uint32_t v, string_result& r)
	{
		size_t pos = size_t(v << 1);
		r.push_back(eco::cast_detail::tables<>::CHAR_100[pos]);
		r.push_back(eco::cast_detail::tables<>::CHAR_100[pos + 1]);
	}*/

	inline void cast_10000(uint32_t v, string_result& r)
	{
		// when v in [0, 9999]:  v / 100 = v * 10486 / 1048576 (<< 20)
		uint32_t tens_div = (v * 10486) >> 20;
		uint32_t tens_mod = v - tens_div * 100;
		cast_100(tens_mod, r);
		cast_100(tens_div, r);
	}

	inline void cast_100000000(uint32_t v, string_result& r)
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
		uint64_t div100 = (merged * 10486) >> 20;
		div100 &= ((0x7Full << 32) | 0x7Full);
		uint64_t mod100 = (merged - div100 * 100);
		cast_100(mod100 & 0x7Full, r);			// YY
		cast_100(div100 & 0x7Full, r);			// XX
		cast_100((mod100 >> 32) & 0x7Full, r);	// BB
		cast_100((div100 >> 32) & 0x7Full, r);	// AA
	}

	inline void cast(uint32_t v, string_result& r)
	{
		/* 
		origin algorithm come from google.absl project.
		https://github.com/abseil/abseil-cpp/blob/master/absl/strings/numbers.h
		1. using cached tables.
		2. using "mutiple & shift" replace "division".
		3. using parallel division.
		*/

		// 10 digits: 0 <= v <= 4294967295
		if (v < 10)
		{
			cast_10(v, r);
		}
		else if (v < 100)
		{
			cast_100(v, r);
		}
		else if (v < 10000)
		{
			cast_10000(v, r);
		}
		else if (v < 100000000)
		{
			cast_100000000(v, r);
		}
		else
		{
			uint32_t div100 = (v / 100);
			uint32_t mod100 = (v % 100);
			cast_100(mod100, r);
			cast_100000000(div100, r);
		}
	}

	inline void cast(uint64_t v, string_result& r)
	{
		// 20 digits: 0 <= v <= 1844 6744 0737 0955 1615
		if (v < 42'9496'7296ull) 				//  10 digits (2^32-1)
		{
			cast(uint32_t(v), r);
		}
		else if (v < 1'0000'0000'0000'0000ull)	// 16 digits
		{
			uint32_t div = uint32_t(v / 1'0000'0000);
			uint32_t mod = uint32_t(v % 1'0000'0000);
			cast_100000000(mod, r);
			cast(div, r);
		}
		else
		{
			// v = AAAA'BBBBBBBB'CCCCCCCC
			uint64_t div08 = uint64_t(v / 1'0000'0000);
			uint32_t mod08 = uint32_t(v % 1'0000'0000);
			uint32_t div16 = uint32_t(div08 / 1'0000);
			uint32_t mod16 = uint32_t(div08 % 1'0000);
			cast_100000000(mod08, r);	// CCCCCCCC
			cast_10000(mod16, r);		// BBBB
			cast_100000000(div16, r);	// AAAABBBB
		}
	}

	// for performance test: 
	// whether divide small integer is more effective than divide big integer
	template<typename uint_t>
	inline void cast_general(uint_t v, string_result& r)
	{
		do
		{
			uint_t div = v / 100;
			uint_t mod = v % 100;
			size_t pos = static_cast<size_t>(mod) << 1;
			r.push_front(eco::cast_detail::tables<>::CHAR_100[pos + 1]);
			r.push_front(eco::cast_detail::tables<>::CHAR_100[pos]);
			v = div;
		} while (v > 0);
	}
};


////////////////////////////////////////////////////////////////////////////////
class integer_to_string
{
	typedef integer_to_string this_t;

	template<typename uint_t>
	inline void do_cast(uint_t v, eco::cast_detail::result& r)
	{
		static_assert(sizeof(uint_t) >= 4);
		r.reset();
		uint32_t shift = 0;
		if (m_format.base < 2 || m_format.base > 36)
		{
			r.fail(true);
		}
		else if (m_format.base == 10)
		{
			// perf: cast ~= cast_general, 
			// perf: cast  < cast_general debug, and cast_general is simple.
			integer_to_string_decimal().cast_general(v, r);
		}
		else if ((shift = to_base_shift(m_format.base)) > 0)
		{
			cast_shift(v, shift, r);	// base = 2^x; 
		}
		else
		{
			cast_number(v, r);			// base != 2^x
		}
	}

public:
	inline void format_result(eco::bool_t negative, eco::cast_detail::result& r)
	{
		r.remove_prefix_zero();

		// prefix: "0X/0B/0"
		if (m_format.base != 10)
		{
			if (m_format.base == 16)
			{
				r.push_front(m_format.int_chars[37]);
				r.push_front('0');
			}
			else if (m_format.base == 8)
			{
				r.push_front('0');
			}
			else if (m_format.base == 2)
			{
				r.push_front(m_format.int_chars[36]);
				r.push_front('0');
			}
		}
		
		// signed: "+/-"
		if (negative)
		{
			r.push_front('-');
		}
		else if (m_format.positive)
		{
			r.push_front('+');
		}

		// placeholder: "0000x123" "   0x123"
		if (m_format.width > 0)
		{
			int hold_size = m_format.width - r.size();
			while (hold_size-- > 0) { r.push_front(m_format.hold); }
		}
	}

	template<typename uint_t>
	inline void cast_shift(
		uint_t v, uint32_t shift, eco::cast_detail::result& r)
	{
		const uint8_t mode_flag = m_format.base - 1;
		do
		{
			uint32_t pos = static_cast<uint32_t>(v & mode_flag);
			r.push_front(m_format.int_chars[pos]);
			v >>= shift;
		} while (v > 0);
	}

	template<typename uint_t>
	inline void cast_number(uint_t v, eco::cast_detail::result& r)
	{
		do
		{
			uint_t div = v / m_format.base;
			uint_t mod = v % m_format.base;
			size_t pos = static_cast<size_t>(mod);
			r.push_front(m_format.int_chars[pos]);
			v = div;
		} while (v > 0);
	}

public:
	inline integer_to_string()
	{}

	inline integer_to_string& format(uint8_t base, uint8_t width, char hold)
	{
		m_format.base = base;
		m_format.width = width;
		m_format.hold = hold;
		return *this;
	}
	inline integer_to_string& lower(eco::bool_t v)
	{
		m_format.lower(v);
		return *this;
	}
	inline integer_to_string& base(uint8_t v)
	{
		m_format.base = v;
		return *this;
	}
	inline integer_to_string& width(uint8_t v)
	{
		m_format.width = v;
		return *this;
	}
	inline integer_to_string& hold(uint8_t v)
	{
		m_format.hold = v;
		return *this;
	}
	inline integer_to_string& positive(eco::bool_t v)
	{
		m_format.positive = (v != 0);
		return *this;
	}

	inline this_t& cast(int32_t v, eco::cast_detail::result& r)
	{
		if (v == INT32_MIN)
		{ 
			r.uint32_min();
			format_result(true, r);
			return *this;
		}
		do_cast<uint32_t>(static_cast<uint32_t>(v < 0 ? -v : v), r);
		if (!r.fail()) { format_result(v < 0, r); }
		return *this;
	}

	inline this_t& cast(uint32_t v, eco::cast_detail::result& r)
	{
		do_cast<uint32_t>(v, r);
		if (!r.fail()) { format_result(0, r); }
		return *this;
	}

	inline this_t& cast(int64_t v, eco::cast_detail::result& r)
	{
		if (v == INT64_MIN) 
		{
			r.uint64_min();
			format_result(true, r);
			return *this;
		}
		do_cast<uint64_t>(static_cast<uint64_t>(v < 0 ? -v : v), r);
		if (!r.fail()) { format_result(v < 0, r); }
		return *this;
	}

	inline this_t& cast(uint64_t v, eco::cast_detail::result& r)
	{
		do_cast<uint64_t>(v, r);
		if (!r.fail()) { format_result(0, r); }
		return *this;
	}

private:
	eco::integer_format m_format;
};


////////////////////////////////////////////////////////////////////////////////
template<typename int_t>
class string_to_integer
{
	static_assert(sizeof(int_t) >= 4);
private:
	inline int base(const char* c, int& b)
	{
		if (b == 0)
		{
			b = 10;
			if (c[0] == '0')
			{
				if (c[1] == 'x' || c[1] == 'X')
				{
					b = 16;
					return 2;
				}
				else if (c[1] == 'b' || c[1] == 'B')
				{
					b = 2;
					return 2;
				}
				else
				{
					b = 8;
					return 1;
				}
			}
		}
		else if (b < 2 || b > 36)
		{
			return -1;
		}
		return 0;
	}

	inline int do_positive(const char* c, const char* e, int base)
	{
		int_t max = std::numeric_limits<int_t>::max();
		int_t over_max = eco::cast_detail::overflow<int_t>::MAX[base - 1];
		// invalid char
		int d = eco::cast_detail::tables<>::to_int(*c);
		if (d >= base) { this->end = c; return -4; }
		for (this->value = d, ++c; c < e; ++c)
		{
			// invalid char
			int d = eco::cast_detail::tables<>::to_int(*c);
			if (d >= base) { this->end = c; return -4; }
			// overflow
			if (this->value > over_max) { this->value = max; return -5; }
			this->value *= static_cast<int_t>(base);
			if (this->value > max - d)  { this->value = max; return -5; }
			this->value += d;
		}
		return 0;
	}

	inline int do_negative(const char* c, const char* e, int base)
	{
		int_t min = std::numeric_limits<int_t>::min();
		int_t over_min = eco::cast_detail::overflow<int_t>::MIN[base - 1];
		// invalid char
		int d = eco::cast_detail::tables<>::to_int(*c);
		if (d >= base) { this->end = c; return -4; }
		for (this->value = -d, ++c; c < e; ++c)
		{
			// invalid char
			int d = eco::cast_detail::tables<>::to_int(*c);
			if (d >= base) { this->end = c; return -4; }
			// overflow
			if (this->value < over_min) { this->value = min; return -5; }
			this->value *= static_cast<int_t>(base);
			if (this->value < min + d)  { this->value = min; return -5; }
			this->value -= d;
		}
		return 0;
	}

	// cast algorithm
	inline int do_cast(const eco::string_view& v, int base = 0)
	{
		// skip whitespace/tab/enter char or v.c_str() is null
		const char* c = v.c_str();
		const char* e = v.c_end();
		for (; c < e && eco::empty(*c); ++c) {}
		if (c >= e) { return -1; }

		// parse negative sign
		int negative =  (*c == '-');
		c += static_cast<int>(*c == '+' || negative);
		if (c >= e) { return -1; }

		// negative number must cast to the signed integer
		// exp: "-33 can't cast to uint32_t"
		if (!eco::signed_int<int_t>() && negative) { return -2; }

		// parse integer base: 16/10/8
		int pos = this->base(c, base);
		if (pos < 0) { return -3; }
		
		// cast number
		return (!negative) ? do_positive(c, e, base) : do_negative(c, e, base);
	}

public:
	inline string_to_integer(const eco::string_view& v, int base = 0)
		: value(0), end(NULL)
	{
		this->error = do_cast(v, base);
	}

	inline eco::bool_t cast(const eco::string_view& v, int base = 0)
	{
		this->value = 0;
		this->end   = NULL;
		this->error = do_cast(v, base);
		return this->error == 0;
	}

	static inline const char* next(const char* c, int base)
	{
		for (; *c != 0; ++c)
		{
			int d = eco::cast_detail::tables<>::to_int(*c);
			if (d < base) { break; }
		}
		return *c == 0 ? NULL : c;
	}

	inline const char* error_message() const
	{
		switch (this->error)
		{
		case -1: return "string number is empty";
		case -2: return "signed number match with unsigned type";
		case -3: return "base must be 0, or in [2, 36]";
		case -4: return "string has invalid number char";
		case -5: return "string number is too large";
		}
	}

	// cast result
	int_t 		value;
	int  	    error;
	const char* end;
};
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);
