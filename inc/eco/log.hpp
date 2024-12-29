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
#define ECO_API
class ECO_API logger
{
public:
    inline static logger& get()
    {

    }

    static inline const char* type()
    {
        return "eco::log::logger";
    }

    virtual void init(eco::log::config& conf);

    virtual void log() = 0;

    void log_args(
        level level, const char* file, int line, const char* title,
        const char* format, va_list* args);
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API plugin
{
public:
    void set_logger(eco::log::logger& );
};


struct logdata
{
    uint64_t reqid;
    uint32_t count;
    const int   line;
    const char* file;
};

struct callstack
{
    const char* func;
};

////////////////////////////////////////////////////////////////////////////////
class stream
    : public eco::stream<eco::log::stream>
    , public eco::format<eco::log::stream>
{
public:
    inline stream(
        level level, const char* file, int line, const char* title,
        const char* format, ...)
    {
        if (logger == NULL)
        {
            printf("please set plugin [eco::log::logger] before use.\n");            
            return;
        }
        tdata.file = file;
        tdata.line = line;
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
    

private:
    int line;
    const char* file;
    static eco::log::logger* logger;
};


////////////////////////////////////////////////////////////////////////////////
#define eco_log_level(level, title, ...) \
  level <= log_level() ? (void)0 : \
  eco::log::stream(level, __FILE__, __LINE__, title, ##__VA_ARGS__, NULL)
#define eco_log_title(level, title, ...) \
  eco::log::level <= log_level() ? (void)0 : \
  eco::log::stream(##level, __FILE__, __LINE__, title, ##__VA_ARGS__, NULL)
#define eco_log_func(level, ...) eco_log(level, __func__, __VA_ARGS__)
#define eco_log(level, ...) eco_log_title(level, 0, ##__VA_ARGS__)

eco_logd();
eco_loge();
eco_logf();
eco_logw();
eco_logi();
////////////////////////////////////////////////////////////////////////////////
} // namespace log
} // namespace eco