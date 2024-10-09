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
eco_namespace(cast);
////////////////////////////////////////////////////////////////////////////////
enum
{
    bool_format_01    = 1,
    bool_format_tf    = 2,
	bool_format_yn    = 3,
};
typedef int bool_format;


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
const char* tables::int_to_char_upper = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char* tables::int_to_char_lower = "0123456789abcdefghijklmnopqrstuvwxyz";
const uint8_t tables::ascii_to_int[80] = {
0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  36, 36, 36, 36, 36, 36, 36, 10, 11, 12, 
13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 
33, 34, 35, 36, 36, 36, 36, 36, 36, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 36, 36, 36, 36};


////////////////////////////////////////////////////////////////////////////////
struct string_result
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
eco_namespace_end(cast);
eco_namespace_end(eco);