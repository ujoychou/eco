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
#include <eco/prec.hpp>
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
inline bool equal(const char* s1, const char* s2)
{
	for (; *s1 && *s2 && *s1 == *s2; ++s1, ++s2) {}
	return *s2 == 0;
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
inline size_t fit(const char* v, size_t size)
{
	size_t i = size - 1;
	for (; i != size_t(-1) && v[i] == 0; --i) {}
	return ++i;
}
inline uint32_t find_first(const char* key, char flag)
{
	uint32_t pos = 0;
	for (; *key != 0 && *key != flag; ++key, ++pos) {}
	return (*key == 0) ? -1 : pos;
}
inline uint32_t find_last(const char* key, uint32_t end, char flag)
{
	const char* it = key + end - 1;
	for (; *it != flag && it >= key; --it) {}
	return static_cast<uint32_t>(it - key);
}
inline uint32_t find_last(const char* key, char flag)
{
	// key format: "logging/file_link/roll_size".
	uint32_t len = static_cast<uint32_t>(strlen(key));
	return find_last(key, len, flag);
}
inline uint32_t find_nth(const char* key, char flag, uint32_t nth)
{
	uint32_t pos = 0;
	uint32_t cur_seq = 0;
	for (; *key != 0; ++key, ++pos)
	{
		if (*key == flag && ++cur_seq == nth) { break; }
	}
	return (*key == 0) ? -1 : pos;
}
inline const char* find(const char* dest, const char* v)
{
	for (; *dest; ++dest)
	{
		if (equal(dest, v)) { return dest; }
	}
	return nullptr;
}


////////////////////////////////////////////////////////////////////////////////
class string_c
{
public:
	inline void append(char c)
	{
		reserve(m_size + 1);
		m_data[m_size++] = c;
		m_data[m_size] = 0;
	}

	inline void append(char c, uint32_t size)
	{
		if (size == 0) return;
		if (size == 1) return append(c);
		reserve(m_size + size);
		memset(&m_data[m_size], c, size);
		m_size += size;
		m_data[m_size] = 0;
	}

	inline void append(const char* str, uint32_t size)
	{
		if (size == 0) return;
		reserve(m_size + size);
		memcpy(&m_data[m_size], str, size);
		m_size += size;
		m_data[m_size] = 0;
	}

public:
	inline string_c() {}

	explicit inline string_c(uint32_t size, bool_t reserved = false)
	{
		reserved ? reserve(size) : resize(size);
	}

	explicit inline string_c(const char* v)
	{
		assign(v);
	}

	explicit inline string_c(const char* v, uint32_t size)
	{
		assign(v, size);
	}

	explicit inline string_c(const std::string& v)
	{
		assign(v.c_str(), static_cast<uint32_t>(v.size()));
	}

	inline string_c(eco::string_c&& v)
		: m_data(v.m_data)
		, m_size(v.m_size)
		, m_capacity(v.m_capacity)
	{
		v.m_data = nullptr;
		v.m_size = 0;
		v.m_capacity = 0;
	}

	inline string_c& operator=(eco::string_c&& v)
	{
		release();
		m_data = v.m_data;
		m_size = v.m_size;
		m_capacity = v.m_capacity;
		v.m_data = nullptr;
		v.m_size = 0;
		v.m_capacity = 0;
		return *this;
	}

	inline string_c& operator=(const char* v)
	{
		assign(v);
		return *this;
	}

	inline string_c& operator=(const std::string& v)
	{
		assign(v);
		return *this;
	}

	inline string_c& operator=(const eco::string_c& v)
	{
		assign(v);
		return *this;
	}

	inline char& operator[](uint32_t pos)
	{
		return m_data[pos];
	}

	inline char operator[](uint32_t pos) const
	{
		return m_data[pos];
	}

	inline ~string_c()
	{
		release();
	}

public:
	inline void swap(eco::string_c& v)
	{
		std::swap(m_data, v.m_data);
		std::swap(m_size, v.m_size);
		std::swap(m_capacity, v.m_capacity);
	}

	inline void assign(const char* v)
	{
		assign(v, static_cast<uint32_t>(strlen(v)));
	}
	inline void assign(const char* v, uint32_t size)
	{
		resize(size);
		if (size > 0) { memcpy(&m_data[0], v, size); }
	}
	inline void assign(const std::string& v)
	{
		assign(v.c_str(), static_cast<uint32_t>(v.size()));
	}
	inline void assign(const eco::string_c& v)
	{
		assign(v.m_data, v.m_size);
	}

	inline void erase(uint32_t pos, uint32_t count)
	{
		if (count > 0 && m_size > pos)
		{
			uint32_t size = m_size - pos;
			if (count < size) { size = count; }
			count = m_size - pos - size;
			if (count > 0)
			{
				memcpy(&m_data[pos], &m_data[pos + size], count);
			}
			m_size -= size;
			m_data[m_size] = 0;
		}
	}

	inline void resize(uint32_t size)
	{
		reserve(size);
		m_size = size;
		if (m_data != nullptr) { m_data[m_size] = 0; }
	}

	inline void reserve(uint32_t c)
	{
		if (m_capacity < c)
		{
			// exponential growth.
			if (c < 32) { c = 32; }
			uint32_t old_size = m_size;
			uint32_t new_size = m_capacity * 2;
			if (new_size < c) { new_size = c; }

			// keep old value.
			char* new_data = (char*)malloc(new_size + 1);
			if (old_size > 0) { memcpy(new_data, m_data, old_size); }
			new_data[old_size] = 0;
			
			release();
			m_data = new_data;
			m_size = old_size;
			m_capacity = new_size;
		}
	}

	inline void release()
	{
		if (m_data != nullptr)
		{
			free(m_data);
			m_data = nullptr;
			m_size = 0;
			m_capacity = 0;
		}
	}

	inline void fit() { resize(uint32_t(eco::fit(m_data, m_size) - 1)); }
	inline void clear() { resize(0); }
	inline const char* c_str() const { return m_data ? m_data : ""; }
	inline uint32_t size() const 	 { return m_size; }
	inline uint32_t capacity() const { return m_capacity; }
	inline eco::bool_t empty() const { return m_data == nullptr || m_size == 0; }

private:
	char*    m_data = nullptr;
	uint32_t m_size = 0;
	uint32_t m_capacity = 0;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);