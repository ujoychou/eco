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
#include <string>
#include <string.h>


eco_namespace(eco)
////////////////////////////////////////////////////////////////////////////////
// char
inline bool_t upper(char v)
{
	return v >= 'A' && v <= 'Z';
}
inline bool_t lower(char v)
{
	return v <= 'z' && v >= 'a';
}
inline bool_t letter(char v)
{
	return upper(v) || lower(v);
}
inline bool_t newline(char v)
{
	return (v == '\n' || v == '\r');
}
inline bool_t space(char v)
{
	return (v == ' ' || v == '	');
}
inline bool_t empty(char v)
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
// number
inline bool_t number(char v)
{
	return v >= '0' && v <= '9';
}
inline bool_t number(const char* v)
{
	for (const char* c = v; *c != 0; ++c)
	{
		if (!number(*c)) { return false; }
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////
// char*
inline bool_t empty(const char* v)
{
	return (v == NULL || v[0] == 0);
}
inline char first(const char* v)
{
	return !empty(v) ? v[0] : 0;
}
inline bool_t iequal(const char* s1, const char* s2, uint32_t size)
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
class string_c
{
public:
	inline void append(char c)
	{
		reserve(this->size + 1);
		this->data[this->size++] = c;
		this->data[this->size] = 0;
	}

	inline void append(char c, uint32_t size)
	{
		if (size == 0) return;
		if (size == 1) return append(c);
		reserve(this->size + size);
		memset(&this->data[this->size], c, size);
		this->size += size;
		this->data[this->size] = 0;
	}

	inline void append(const char* str, uint32_t size)
	{
		if (size == 0) return;
		reserve(this->size + size);
		memcpy(&this->data[this->size], str, size);
		this->size += size;
		this->data[this->size] = 0;
	}

public:
	inline string_c()
		: data(nullptr), size(0), capacity(0)
	{}

	explicit inline string_c(uint32_t size, bool_t reserved = false)
		: data(nullptr), size(0), capacity(0)
	{
		reserved ? reserve(size) : resize(size);
	}

	explicit inline string_c(const char* v)
		: data(nullptr), size(0), capacity(0)
	{
		assign(v);
	}

	explicit inline string_c(const char* v, uint32_t size)
		: data(nullptr), size(0), capacity(0)
	{
		assign(v, size);
	}

	explicit inline string_c(const std::string& v)
		: data(nullptr), size(0), capacity(0)
	{
		assign(v.c_str(), (uint32_t)v.size());
	}

	inline string_c(eco::string_c&& v)
		: data(v.data), size(v.size), capacity(v.capacity)
	{
		v.data = nullptr;
		v.size = 0;
		v.capacity = 0;
	}

	inline ~string_c()
	{
		release();
	}

public:
	inline void swap(eco::string_c& v)
	{
		std::swap(this->data, v.data);
		std::swap(this->size, v.size);
		std::swap(this->capacity, v.capacity);
	}

	inline void assign(const char* v)
	{
		assign(v, static_cast<uint32_t>(strlen(v)));
	}
	inline void assign(const char* v, uint32_t size)
	{
		resize(size);
		if (size > 0) { memcpy(&this->data[0], v, size); }
	}
	inline void assign(const std::string& v)
	{
		assign(v.c_str(), (uint32_t)v.size());
	}

	inline void erase(uint32_t pos, uint32_t count)
	{
		if (count > 0 && this->size > pos)
		{
			uint32_t size = this->size - pos;
			if (count < size) { size = count; }
			count = this->size - pos - size;
			if (count > 0)
			{
				memcpy(&this->data[pos], &this->data[pos + size], count);
			}
			this->size -= size;
			this->data[this->size] = 0;
		}
	}

	inline void resize(uint32_t size)
	{
		reserve(size);
		this->size = size;
		if (this->data != nullptr) { this->data[this->size] = 0; }
	}

	inline void reserve(uint32_t c)
	{
		if (this->capacity < c)
		{
			// exponential growth.
			if (c < 32) { c = 32; }
			uint32_t old_size = this->size;
			uint32_t new_size = this->capacity * 2;
			if (new_size < c) { new_size = c; }

			// keep old value.
			char* new_data = (char*)malloc(new_size + 1);
			if (old_size > 0) { memcpy(new_data, this->data, old_size); }
			new_data[old_size] = 0;
			
			release();
			this->data = new_data;
			this->size = old_size;
			this->capacity = new_size;
		}
	}

	inline void fit()
	{
		resize(uint32_t(eco::fit(this->data, this->size) - 1));
	}

	inline void clear()
	{
		resize(0);
	}

	inline void release()
	{
		if (this->data != nullptr)
		{
			free(this->data);
			this->data = nullptr;
			this->size = 0;
			this->capacity = 0;
		}
	}

private:
	char*    data;
	uint32_t size;
	uint32_t capacity;
};

////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);