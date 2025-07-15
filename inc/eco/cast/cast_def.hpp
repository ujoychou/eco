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
#include <limits>


eco_namespace(eco);
eco_namespace(cast_detail);
////////////////////////////////////////////////////////////////////////////////
template<typename T = void>
struct tables
{
	static const char* CHAR_100;
	static const char* CHAR_UPPER;
	static const char* CHAR_LOWER;
	static const uint8_t ASCII_TO_INT[80];

	// char to int
	static inline uint8_t to_int(char c)
	{
		return (c < '0' || c > 'z') ? 36 : ASCII_TO_INT[c - '0'];
	}
};
// string size = 200
template<typename T>
const char* tables<T>::CHAR_100 = "00010203040506070809"
"101112131415161718192021222324252627282930313233343536373839"
"404142434445464748495051525354555657585960616263646566676869"
"707172737475767778798081828384858687888990919293949596979899";
// string size = 36
template<typename T>
const char* tables<T>::CHAR_UPPER = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZBX";
template<typename T>
const char* tables<T>::CHAR_LOWER = "0123456789abcdefghijklmnopqrstuvwxyzbx";
template<typename T>
const uint8_t tables<T>::ASCII_TO_INT[80] = {
0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  36, 36, 36, 36, 36, 36, 36, 10, 11, 12, 
13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 
33, 34, 35, 36, 36, 36, 36, 36, 36, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 36, 36, 36, 36};


////////////////////////////////////////////////////////////////////////////////
// number overflow talbe = 36
#define eco_cast_overflow_init(X) \
{\
	0, X/2, X/3, X/4, X/5, X/6, X/7, X/8, X/9, X/10, X/11, X/12, X/13, X/14,\
	X/15, X/16, X/17, X/18, X/19, X/20, X/21, X/22, X/23, X/24, X/25, X/26,\
	X/27, X/28, X/29, X/30, X/31, X/32, X/33, X/34, X/35, X/36\
}
template<typename int_t>
struct overflow
{
	static const int_t MAX[36];
	static const int_t MIN[36];
};
template <typename int_t>
const int_t overflow<int_t>::MAX[36] = 
eco_cast_overflow_init(std::numeric_limits<int_t>::max());
template <typename int_t>
const int_t overflow<int_t>::MIN[36] = 
eco_cast_overflow_init(std::numeric_limits<int_t>::min());


////////////////////////////////////////////////////////////////////////////////
struct result
{
public:
	static const uint32_t capacity = 36;

	inline result() : pos(capacity - 1)
	{
		buff[0] = 0;
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

	inline result& fail(bool_t v)
	{
		buff[0] = (char)v;
		return *this;
	}

	inline bool fail() const
	{
		return (buff[0] == 1);
	}

public:
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
eco_namespace_end(cast_detail);


////////////////////////////////////////////////////////////////////////////////
enum bool_format
{
    bool_format_01    = 1,
    bool_format_tf    = 2,
	bool_format_yn    = 3,
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);