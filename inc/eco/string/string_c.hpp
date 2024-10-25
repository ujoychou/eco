#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-08-21] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/macro.hpp>
#include <stdio.h>


eco_namespace(eco)
////////////////////////////////////////////////////////////////////////////////
inline bool upper(char v)
{
	return v >= 'A' && v <= 'Z';
}
inline bool lower(char v)
{
	return v <= 'z' && v >= 'a';
}
inline bool letter(char v)
{
	return upper(v) || lower(v);
}
inline bool newline(char v)
{
	return (v == '\n' || v == '\r');
}
inline bool space(char v)
{
	return (v == ' ' || v == '	');
}
inline bool empty(char v)
{
	return space(v) || newline(v);
}
////////////////////////////////////////////////////////////////////////////////
inline char to_upper(char v)
{
	return lower(v) ? v + ('A' - 'a') : v;
}
inline char to_lower(char v)
{
	return upper(v) ? v + ('a' - 'A') : v;
}


////////////////////////////////////////////////////////////////////////////////
inline bool number(char v)
{
	return v >= '0' && v <= '9';
}
inline bool number(const char* v)
{
	for (const char* c = v; *c != 0; ++c)
	{
		if (!number(*c)) { return false; }
	}
	return true;
}
inline bool big_endian()
{
	uint32_t v = 1;
	return *reinterpret_cast<char*>(&v) == 1;
}
inline bool little_endian()
{
	return !big_endian();
}
////////////////////////////////////////////////////////////////////////////////
template<typename integer_t>
inline integer_t to_negative(integer_t v)
{
	return v <= 0 ? v : (0 - v);
}
template<typename integer_t>
inline integer_t to_positive(integer_t v)
{
	return v >= 0 ? v : (0 - v);
}
inline uint32_t to_base_shift(uint32_t base)
{
	uint32_t shift = 1;
	if (base < 2 || (base & (base - 1)) != 0) { return 0; }
	for (; base > 2; base >>= 1, ++shift) {}
	return shift;
}


////////////////////////////////////////////////////////////////////////////////
inline bool empty(const char* v)
{
	return (v == NULL || v[0] == 0);
}
inline char first(const char* v)
{
	return !empty(v) ? v[0] : 0;
}
inline bool iequal(const char* s1, const char* s2, uint32_t size)
{
	assert(size > 0);
	char c1, c2;
	do 
	{
		c1 = to_upper(*s1++);
		c2 = to_upper(*s2++);
	} while (c1 == c2 && c1 != 0 && --size > 0);
	return c1 == c2;
}
#ifdef eco_win32
inline int snprintf(char* buff, size_t size, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int result = _vsnprintf(buff, size, format, args);
	va_end(args);
	return result;
}
#endif
////////////////////////////////////////////////////////////////////////////////
inline char* to_upper(char* v)
{
	for (char* c = v; *c != 0; ++c) { *c = to_upper(*c); }
	return v;
}
inline char* to_lower(char* v)
{
	for (char* c = v; *c != 0; ++c) { *c = to_lower(*c); }
	return v;
}
// clear the string data.
inline void clear(char* v)
{
	v[0] = 0;
}
// get string end size.
inline size_t fit(const char* str, size_t size)
{
	size_t i = size - 1;
	for (; i != size_t(-1) && str[i] == 0; --i) {}
	return ++i;
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);