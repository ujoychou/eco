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
private:
    int option;
    // error key: id & path
    int id;
    eco::string path;

    // error message: "message / params of message"
    eco::string message;
    const char* format;

    friend class eco::error;
};
eco_namespace_end(detail);


////////////////////////////////////////////////////////////////////////////////
enum class result : int
{
    ok 	        = 0,
    fail        = 1,
    error	    = 2,
    timeout     = 3,

    syserr      = 0x1 << 31,
};


////////////////////////////////////////////////////////////////////////////////
class error : public eco::stream<eco::error>, public eco::format<eco::error>
{
public:
    inline error() : data(this_thread_data())
    {}

    inline error(int id, const char* format) : data(this_thread_data())
    {
        data.id = id;
        data.path.clear();
        data.format = format;
    }

    inline error(const char* path, const char* format)
        : data(this_thread_data())
    {
        data.id = 0;
        data.path = path;
        data.format = format;
        //data.message.format(format, );
    }

    inline error& operator % (int v)
    {
        return (*this);
    }

    inline const eco::string& message()
    {
        return data.message;
    }

    inline eco::error& sys(bool_t value)
    {
        if (value)
            data.id |= eco::result::syserr;
        else
            data.id &= ~eco::result::syserr;
    }
    inline bool sys() const
    {
        return (data.id & eco::result::syserr) != 0;
    }

private:
    eco::detail::error_data& data;
    static eco::detail::error_data& this_thread_data();
};


////////////////////////////////////////////////////////////////////////////////
// return error mode: set error information in sub function
#define eco_error(...) eco::error(##__VA_ARGS__, NULL)

// throw error mode: throw error(a spice) object in sub function
#define eco_throw(...) throw eco::error(##__VA_ARGS__, NULL)

////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);