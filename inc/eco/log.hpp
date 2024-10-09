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
#include <stdint.h>
#include <stdarg.h>
#include <eco/string/stream.hpp>


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
    trace	= 11,
    info	= 12,
    warn	= 13,
    error	= 14,
    fatal	= 15,
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
class stream : public eco::stream<eco::log::stream>
{
public:
    inline stream(
        level level, const char* file, int line, const char* title,
        const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        log_args(level, file, line, title, format, &args);
        va_end(args);
    }

    // format: "time thread [level] <title> message ... (@file:line)"
    void log_args(
        level level, const char* file, int line, const char* title,
        const char* format, va_list* args);
};


////////////////////////////////////////////////////////////////////////////////
#define eco_log_level(level, title, ...) \
  level <= log_level() ? (void)0 : \
  eco::log::stream(level, __FILE__, __LINE__, title, ##__VA_ARGS__, NULL)
#define eco_log(level, title, ...) \
  eco::log::level <= log_level() ? (void)0 : \
  eco::log::stream(##level, __FILE__, __LINE__, title, ##__VA_ARGS__, NULL)

#define eco_trace(...) eco_log(trace, 0, ##__VA_ARGS__)
#define eco_debug(...) eco_log(debug, 0, ##__VA_ARGS__)
#define eco_info(...)  eco_log(info,  0, ##__VA_ARGS__)
#define eco_warn(...)  eco_log(warn,  0, ##__VA_ARGS__)
#define eco_error(...) eco_log(error, 0, ##__VA_ARGS__)
#define eco_fatal(...) eco_log(fatal, 0, ##__VA_ARGS__)
#define eco_func(level, ...) eco_log(level, __func__, __VA_ARGS__)
////////////////////////////////////////////////////////////////////////////////
} // namespace log
} // namespace eco