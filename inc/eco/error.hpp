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
#include <eco/string/string.hpp>


////////////////////////////////////////////////////////////////////////////////
// error key: id & path
eco_namespace(eco);
eco_namespace(detail);
struct error_data
{
    int id;
    eco::string path;
    eco::string message;
    const char* format;
};
eco_namespace_end(detail);


////////////////////////////////////////////////////////////////////////////////
// thread local error data
eco_namespace(this_thread);
eco::detail::error_data& error();
eco_namespace_end(this_thread);


////////////////////////////////////////////////////////////////////////////////
class error : public eco::stream<eco::error>
{
public:
    inline error(int id) : data(eco::this_thread::error())
    {
        data.id = id;
    }

    inline error(const char* path) : data(eco::this_thread::error())
    {
        data.path = path;
    }

    inline error(int id, const char* format, ...)
        : data(eco::this_thread::error())
    {
        data.id = id;
        data.format = format;
    }

    inline error(const char* path, const char* format, ...)
        : data(eco::this_thread::error())
    {
        data.path = path;
        data.format = format;
    }

    inline type_t& operator % (int v)
    {
        return (type_t&)(*this);
    }

    inline const eco::string& message()
    {
        return data.message;
    }

private:
    eco::detail::error_data& data;
};


#define eco_throw(...) throw eco::error(##__VA_ARGS__)
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);