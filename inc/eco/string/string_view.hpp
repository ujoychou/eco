#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-08-24] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/prec.hpp>
#include <string.h>


eco_namespace(eco)
////////////////////////////////////////////////////////////////////////////////
class string_view
{
public:
	inline string_view(const char* v = nullptr) : m_data(v), m_size(-1)
	{}

	inline string_view(const char* v, uint32_t size) : m_data(v), m_size(size)
	{}

	template<typename string_t>
	explicit inline string_view(const string_t& v) 
		: m_data(v.c_str()), m_size(static_cast<uint32_t>(v.size()))
	{}

	inline void reset(const char* v, uint32_t size)
	{
		m_data = v;
		m_size = size;
	}

	inline bool null() const
	{
		return (m_data == NULL);
	}

	inline const char* c_str() const
	{
		return m_data;
	}

	inline const char* c_end() const
	{
		return m_data + size();
	}

	inline char operator[](uint32_t index) const
	{
		return null() ? 0 : m_data[index];
	}

	inline uint32_t size() const
	{
		if (m_size == (uint32_t)-1) { m_size = !null() ? strlen(m_data) : 0; }
		return m_size;
	}

	inline string_view& size(uint32_t v)
	{
		m_size = v;
		return *this;
	}

protected:
	const char*      m_data;
	mutable uint32_t m_size;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco)