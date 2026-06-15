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
#include <eco/string/stream.hpp>
#include <eco/string/string_view.hpp>


eco_namespace(eco)
////////////////////////////////////////////////////////////////////////////////
class string_entry : public eco::string_view, public eco::stream_t<string_entry>
{
public:
	inline string_entry(char* v = NULL) : eco::string_view((const char*)v)
	{}

	inline string_entry(char* v, uint32_t size) : eco::string_view((const char*)v, size)
	{}

	template<typename string_t>
	explicit inline string_entry(const string_t& v) : eco::string_view(v)
	{}

	inline void reset(char* data, uint32_t capacity)
    {
        eco::string_view::reset(data, capacity);
    }

    inline uint32_t left() const
    {
        return m_size - m_pos;
    }

    inline uint32_t size_no_overflow(uint32_t size) const
    {
        uint32_t size_left = left();
        return (size_left < size ? size_left : size);
    }

	inline void append(char c, uint32_t size)
	{
		size = size_no_overflow(size);
		if (size == 0) return;
		if (size == 1) return append(c);
		memset(&data()[m_size], c, size);
		m_size += size;
	}

	inline void append(const char* str, uint32_t size)
	{
		size = size_no_overflow(size);
		if (size == 0) return;
		memcpy(&data()[m_size], str, size);
		m_size += size;
	}

	inline void append(const char* str)
	{
	}

	inline const char* text()
	{
		return "";
	}

private:
	inline void append(char c)
	{
		data()[m_pos++] = c;
	}

	inline char* data()
	{
		return const_cast<char*>(m_data);
	}

	uint32_t m_pos = 0;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco)