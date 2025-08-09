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
#include <eco/string/format.hpp>
#include <eco/plugin/logger.hpp>


namespace eco {
namespace log {
////////////////////////////////////////////////////////////////////////////////
class factory
{
public:
    static eco::plugin::logger& logger();
};


////////////////////////////////////////////////////////////////////////////////
class stream
    : public eco::stream<eco::log::stream>
    , public eco::format<eco::log::stream>
{
public:
    inline void log(
        const char* file, int line,
        level level, const char* title,
        const char* format, ...)
    {
        if (logger == NULL)
        {
            printf("please set plugin [eco::log::logger] before use.\n");            
            return;
        }
        if (format == NULL)
        {

            return;
        }
        

        va_list args;
        va_start(args, format);
        logger->log(level, file, line, title, format, &args);
        va_end(args);
    }

    inline stream(
        level level, const char* file, int line, const char* title,
        bool_t when, uint32_t each, const char* format, ...)
    {
        if (when) { return; }
        if (!logger->each(file, line, each)) { return; }
    }

    inline stream& when(bool_t islog)
    {
        if (!islog)
        {
            this->file = NULL;            
        }
    }
    inline stream& each(uint32_t nth)
    {
        return when(logger->each(file, line, nth));
    }

    // format: "time thread [level] <title> message ... (@file:line)"
};


////////////////////////////////////////////////////////////////////////////////
#define eco_log_level(level, title, ...) \
  level <= log_level() ? (void)0 : \
  eco::log::stream(level, __FILE__, __LINE__, title, ##__VA_ARGS__, NULL)
#define eco_log_title(level, title, ...) \
  eco::log::level <= log_level() ? (void)0 : \
  eco::log::stream(eco::log::level, __FILE__, __LINE__, title, ##__VA_ARGS__, NULL)
#define eco_log_func(level, ...) eco_log(level, __func__, __VA_ARGS__)
#define eco_log(level, ...) eco_log_title(level, 0, ##__VA_ARGS__)


#define eco_trace(...) eco_log(trace, 0, ##__VA_ARGS__)
#define eco_debug(...) eco_log(debug, 0, ##__VA_ARGS__)
#define eco_info(...)  eco_log(info,  0, ##__VA_ARGS__)
#define eco_warn(...)  eco_log(warn,  0, ##__VA_ARGS__)
#define eco_error(...) eco_log(error, 0, ##__VA_ARGS__)
#define eco_fatal(...) eco_log(fatal, 0, ##__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
} // namespace log
} // namespace eco



int main()
{
    eco::App::get().plugin()::create();
    eco::log::plugin logg = eco::app().plugin().load("eco::log::logger");
    return 0;
}