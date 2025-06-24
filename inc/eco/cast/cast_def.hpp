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


eco_namespace(eco);
eco_namespace(cast_detail);
////////////////////////////////////////////////////////////////////////////////
struct tables
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
const char* tables::int_to_char_100 = "00010203040506070809"
"101112131415161718192021222324252627282930313233343536373839"
"404142434445464748495051525354555657585960616263646566676869"
"707172737475767778798081828384858687888990919293949596979899";
const char* tables::int_to_char_upper = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZBX";
const char* tables::int_to_char_lower = "0123456789abcdefghijklmnopqrstuvwxyzbx";
const uint8_t tables::ascii_to_int[80] = {
0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  36, 36, 36, 36, 36, 36, 36, 10, 11, 12, 
13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 
33, 34, 35, 36, 36, 36, 36, 36, 36, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 36, 36, 36, 36};
eco_namespace_end(cast_detail);


////////////////////////////////////////////////////////////////////////////////
enum bool_format
{
    bool_format_01    = 1,
    bool_format_tf    = 2,
	bool_format_yn    = 3,
};


////////////////////////////////////////////////////////////////////////////////
struct cast_result
{
public:
	static const uint32_t capacity = 36;

	inline cast_result() : pos(capacity - 1)
	{
		buff[pos] = 0;
	}

	inline uint32_t size() const
	{
		return (capacity - 1 - pos);
	}

	inline const char* c_str() const
	{
		return &this->buff[0];
	}

	inline cast_result& fail(bool_t v)
	{
		buff[0] = (char)v;
		return *this;
	}

	inline bool fail() const
	{
		return (buff[0] == 1);
	}

protected:
	friend class eco::integer_to_string_decimal;
	friend class eco::integer_to_string;
	friend class eco::cast_detail::double_to_string_result;

	inline void push_front(char c)
	{
		buff[--pos] = c;
	}

	inline void push_back(char c)
	{
		buff[pos++] = c;
	}

	inline void push_back(const char* c)
	{
		for (; *c != 0; buff[pos++] = *c);
	}

	inline void pop_front_zero()
	{
		for (; buff[pos] == 0 && pos < (capacity - 1); ++pos);
	}

	uint32_t 	pos;
	char 		buff[capacity];
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);