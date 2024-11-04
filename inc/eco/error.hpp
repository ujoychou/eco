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
eco_namespace(eco);
eco_namespace(detail);
struct error_data
{
    // error key: id & path
    int option;
    int id;
    eco::string path;
    // error message: "message / params of message"
    eco::string message;
    const char* format;
};
eco_namespace_end(detail);


////////////////////////////////////////////////////////////////////////////////
enum
{
    result_true 	    = 0,
    result_false        = 1,
    result_error	    = 2,
    result_timeout      = 3,
};
typedef int result;


////////////////////////////////////////////////////////////////////////////////
class error : public eco::stream<eco::error>
{
public:
    inline error() : data(this_thread_data())
    {}

    inline error(int id, const char* format, ...) : data(this_thread_data())
    {
        data.id = id;
        data.path.clear();
        data.format = format;
    }

    inline error(const char* path, const char* format, ...)
        : data(this_thread_data())
    {
        data.id = 0;
        data.path = path;
        data.format = format;
        data.message.format(format, );
    }

    inline error& operator % (int v)
    {
        return (*this);
    }

    inline const eco::string& message()
    {
        return data.message;
    }

    inline error& sys(bool_t value)
    {
        if (value)
            data.id |= 0x1000000;
        else
            data.id &= ~0x1000000;
    }
    inline bool sys() const
    {
        return (data.id & 0x1000000) != 0;
    }

private:
    static eco::detail::error_data& this_thread_data();

private:
    eco::detail::error_data& data;
};


////////////////////////////////////////////////////////////////////////////////
#define eco_throw(...) throw eco::error(##__VA_ARGS__, NULL)
#define eco_trace(...) eco::error(##__VA_ARGS__, NULL)
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);