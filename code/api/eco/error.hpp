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
#include <eco/string/stream.hpp>
#include <eco/string/string.hpp>


////////////////////////////////////////////////////////////////////////////////
eco_namespace(eco);
eco_namespace(data);
struct error
{
    int id;
    eco::string path;
    eco::string message;
};
eco_namespace_end(data);

eco_namespace(this_thread);
eco::data::error& error();
eco_namespace_end(this_thread);



template<typename type_t>
class format
{
public:
    inline type_t& rthis() { return (type_t&)(*this); }

    inline type_t& operator % (int v)
    {
        return (type_t&)(*this);
    }
};

////////////////////////////////////////////////////////////////////////////////
class error : 
    public std::exception,
    public eco::stream<eco::error>,
    public eco::format<eco::error>
{
public:
    inline error(int id) : data(eco::this_thread::error()) {}
    inline error(int id, const char* format, ...);
    inline error(const char* path);
    inline error(const char* path, const char* format, ...);

private:
    eco::data::error& data;
};


#define eco_throw(...) throw eco::error(##__VA_ARGS__)
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);