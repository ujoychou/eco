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
#include <eco/string/stream.hpp>
#include <eco/string/format.hpp>


eco_namespace(eco)
////////////////////////////////////////////////////////////////////////////////
class string 
	: public eco::stream<eco::string>
	, public eco::format<eco::string>
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
	inline string()	: m_data(nullptr), m_size(0), m_capacity(0)
	{}

	explicit inline string(uint32_t size, bool reserved = false)
		: m_data(nullptr), m_size(0), m_capacity(0)
	{
		if (reserved)
			reserve(size);
		else
			resize(size);
	}

	explicit inline string(const char* v)
		: m_data(nullptr), m_size(0), m_capacity(0)
	{
		assign(v);
	}

	explicit inline string(const char* v, uint32_t size)
		: m_data(nullptr), m_size(0), m_capacity(0)
	{
		assign(v, size);
	}

	explicit inline string(const std::string& v)
		: m_data(nullptr), m_size(0), m_capacity(0)
	{
		assign(v.c_str(), (uint32_t)v.size());
	}

	inline string(eco::string&& v)
		: m_data(v.m_data)
		, m_size(v.m_size)
		, m_capacity(v.m_capacity)
	{
		v.m_data = nullptr;
		v.m_size = 0;
		v.m_capacity = 0;
	}

	inline ~string()
	{
		release();
	}

public:
	inline string& operator=(eco::string&& v)
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

	inline string& operator=(const char* v)
	{
		assign(v);
		return *this;
	}

	inline string& operator=(const std::string& v)
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

	inline void swap(string& v)
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
		assign(v.c_str(), (uint32_t)v.size());
	}

	inline void erase(uint32_t pos, uint32_t count)
	{
		if (count > 0 && m_size > pos)
		{
			uint32_t size = m_size - pos;
			if (count < size) { size = count; }
			count = m_size - pos - size;
			if (count > 0) { memcpy(&m_data[pos], &m_data[pos + size], count); }
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

	inline void fit()
	{
		resize(uint32_t(eco::fit(m_data, m_size) - 1));
	}

	inline void clear()
	{
		resize(0);
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

private:
	char* m_data;
	uint32_t m_size;
	uint32_t m_capacity;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco)