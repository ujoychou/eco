

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
#include <eco/prec.hpp>
#include <stdarg.h>
#include <string.h>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
struct entry
{
    explicit inline entry(void* data = 0) : m_buff(static_cast<char*>(data))
    {}

    inline void reset(void* data, uint32_t capacity)
    {
        m_buff = static_cast<char*>(data);
        m_pos_current = 0;
        m_pos_message = 0;
        m_capacity = capacity;
    }

    inline void set_message_pos()
    {
        m_pos_message = m_pos_current;
    }

    inline uint32_t left() const
    {
        return m_capacity - m_pos_current;
    }

    inline uint32_t adjust(uint32_t size) const
    {
        uint32_t size_left = left();
        return (size_left < size ? size_left : size);
    }

    inline void printf(const char* format, va_list& args)
    {
        snprintf(m_buff, left(), format, args);
    }

    inline entry& append(char c)
	{
		m_buff[m_pos_current++] = c;
		m_buff[m_pos_current] = 0;
		return *this;
	}

    inline entry& append(char c, uint32_t size)
    {
        size = adjust(size);
		if (size == 1) return append(c);
		memset(&m_buff[m_pos_current], c, size);
		m_pos_current += size;
		m_buff[m_pos_current] = 0;
		return *this;
    }

    inline entry& append(const char* str, uint32_t size = 0)
    {
        if (size == 0) { size = strlen(str); }
        size = adjust(size);
		memcpy(&m_buff[m_pos_current], str, size);
		m_pos_current += size;
		m_buff[m_pos_current] = 0;
		return *this;
    }

    inline const char* text() const
    {
        return m_buff;
    }

    inline const char* message() const
    {
        return m_buff + m_pos_message;
    }

protected:
    uint32_t m_pos_current = 0;
    uint32_t m_pos_message = 0;
    uint32_t m_capacity = 0;
    char*    m_buff = nullptr;
};


////////////////////////////////////////////////////////////////////////////////
class cache
{
public:
    static void init_logs(uint32_t entry_size, uint32_t count);

    static eco::entry& entry_this_error();

    static eco::entry entry_this_borrow();

    static void entry_this_return(eco::entry& entry);
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);