#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2025-08-07] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <stdint.h>
#include <stdarg.h>
#include <eco/string/stream.hpp>
#include <eco/string/format.hpp>


namespace eco {
namespace log {
////////////////////////////////////////////////////////////////////////////////
enum
{
	none    = 0,
    l1      = 1,
    l2      = 2,
    l3      = 3,
    l4      = 4,
    l5      = 5,
    l6      = 6,
    l7      = 7,
    l8      = 8,
    l9      = 9,
    // log general level
    debug	= 10,
    info	= 11,
    warn	= 12,
    error	= 13,
    fatal	= 14,
};
typedef int level;

enum 
{
	// log message persist in file
	file    = 0x01,
	// log message display on console
	console	= 0x02,
	// log message send to monitor server
	monitor	= 0x04,
};
typedef int notify;


////////////////////////////////////////////////////////////////////////////////
class config
{
public:
    level level(void);

    void  init(
        int console, 
        const char* directory,
        const char* filename,
        int log_level,
        uint32_t interval,
        uint32_t size);
};


////////////////////////////////////////////////////////////////////////////////
class eco_api plugin_logger
{
public:
    static inline const char* type()
    {
        return "eco::log::logger";
    }

    virtual void init(
        eco::log::config& conf) = 0;

    virtual void log(
        const char* file, int line,
        level level, const char* title,
        const char* format, va_list* args);
};


////////////////////////////////////////////////////////////////////////////////
} // namespace log
} // namespace eco