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
#include <eco/string.hpp>
#include <eco/string/string_entry.hpp>
#include <eco/thread.hpp>
#include <eco/datetime.hpp>
#include <eco/log/chain.hpp>


eco_namespace(eco);
eco_namespace(log);
////////////////////////////////////////////////////////////////////////////////
enum
{
    // for developer
    none    = 0,
    debug	= 11,
    trace	= 13,

    // for user
    info	= 23,
    key	    = 27,
    warn	= 31,

    // for system error
    error	= 41,
    bug  	= 43,       // program bugs
    sys	    = 45,       // system error
    env	    = 47,       // hardware error

    // for fatal error
    fatal	= 53,
};
typedef uint8_t level;

// modula id: [1, 255], modula size config in <logger>
typedef uint8_t  modula;
const modula modula_all = 0;
const modula modula_max = 255;

// communication mode
enum
{
    msg     = 0,        // :: message
	req     = 1,        // -- request
	rsp     = 2,        // == reply
	pub     = 3,        // >> publish
	sub     = 4,        // << subscribe
};
typedef uint8_t mode;

// int level (4byte) = modula (2byte) + level (1byte) + mode (1byte)
inline int level_modula(int level)
{
    return level & 0x00FF0000;
}
inline int level_level(int level)
{
    return level & 0x000000FF;
}
inline int level_mode(int level)
{
    return level & 0xFF000000;
}
inline int level_mode(int level, int mode)
{
    return level | (mode << 24);
}


////////////////////////////////////////////////////////////////////////////////
struct message
{
    int                 level;
    int                 line;
    const char*         file;
    const char*         func;
    const char*         aspect;
    const char*         user;
    eco::string_entry   entry;
    
    inline message(int level, int line, const char* file,  const char* func)
        : level(level), line(line), file(file), func(func), aspect(0), user(0)
    {}
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);