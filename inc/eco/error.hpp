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
#include <eco/string.hpp>
#include <eco/cache/cache.hpp>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
class error : 
    public eco::format_t<eco::error>,
    public eco::stream_t<eco::error>
{
public:
    inline error(int id, const char* file, int line, const char* format, ...)
        : m_buff(eco::cache::entry_this_error())
    {}

    inline error(const char* path, const char* file, int line, const char* format, ...)
         : m_buff(eco::cache::entry_this_error())
    {}

    inline void printf(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        m_buff.printf(format, args);
        va_end(args);
    }

private:
    eco::entry m_buff;
};


////////////////////////////////////////////////////////////////////////////////
// return error mode: set error information in sub function
#define eco_error(id_or_path, ...) \
eco::error(id_or_path, __func__, __LINE__, ##__VA_ARGS__, NULL)

// throw error mode: throw error(a spice) object in sub function
#define eco_throw_1(id_or_path) \
throw eco::error(id_or_path, __func__, __LINE__, NULL)
#define eco_throw_2(cond, id_or_path) \
for (int w = 1; w && cond; w = 0) \
    throw eco::error(id_or_path, __func__, __LINE__, NULL)
#define eco_throw(...) eco_macro_overload(eco_throw_,__VA_ARGS__)
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);