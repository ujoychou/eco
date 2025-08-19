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
#include <stdarg.h>
#include <eco/string.hpp>
#include <eco/thread.hpp>
#include <eco/datetime.hpp>
#include <eco/rtti.hpp>


eco_namespace(eco);
eco_namespace(log);
////////////////////////////////////////////////////////////////////////////////
enum
{
	none    = 0x0000,
    // log level set mode: value="l1,l4,l7", it will ouput l1, l4, l7.
    l1      = 0x0001,
    l2      = 0x0002,
    l3      = 0x0004,
    l4      = 0x0008,
    l5      = 0x0010,
    l6      = 0x0020,
    l7      = 0x0040,
    l8      = 0x0080,
    l9      = 0x0100,
    la      = 0x0200,
    lb      = 0x0400,
    // log level min mode: value="warn", it will ouput warn/error/fatal
    debug	= 0x0800,
    info	= 0x1000,
    warn	= 0x2000,
    error	= 0x4000,
    fatal	= 0x8000,
};
typedef int level;

enum
{
    on_begin	= 0x0001,
    on_end		= 0x0002,
    on_title	= 0x0003,
};
typedef int on_time;

////////////////////////////////////////////////////////////////////////////////
struct entry
{
    explicit inline entry(void* data = 0) : m_buff(static_cast<char*>(data))
    {}

    inline void reset(void* data, uint32_t capacity)
    {
        m_buff = static_cast<char*>(data);
        m_pos_current = 0;
        m_pos_message = 0;
        m_capacity = capacity;
    }

    inline void set_message_pos()
    {
        m_pos_message = m_pos_current;
    }

    inline uint32_t left() const
    {
        return m_capacity - m_pos_current;
    }

    inline uint32_t adjust(uint32_t size) const
    {
        uint32_t size_left = left();
        return (size_left < size ? size_left : size);
    }

    inline void printf(const char* format, va_list* args)
    {
        snprintf(m_buff, left(), format, *args);
    }

    inline entry& append(char c)
	{
		m_buff[m_pos_current++] = c;
		m_buff[m_pos_current] = 0;
		return *this;
	}

    inline entry& append(char c, uint32_t size)
    {
        size = adjust(size);
		if (size == 1) return append(c);
		memset(&m_buff[m_pos_current], c, size);
		m_pos_current += size;
		m_buff[m_pos_current] = 0;
		return *this;
    }

    inline entry& append(const char* str, uint32_t size)
    {
        size = adjust(size);
		memcpy(&m_buff[m_pos_current], str, size);
		m_pos_current += size;
		m_buff[m_pos_current] = 0;
		return *this;
    }

    inline const char* text() const
    {
        return m_buff;
    }

    inline const char* message() const
    {
        return m_buff + m_pos_message;
    }

protected:
    uint32_t m_pos_current = 0;
    uint32_t m_pos_message = 0;
    uint32_t m_capacity = 0;
    char*    m_buff = nullptr;
};


////////////////////////////////////////////////////////////////////////////////
struct message
{
    eco::log::level  level;
    eco::datetime    time;
    const char*      thread;
    const char*      tid;
    eco::bool_t      when;
    int              line;
    const char*      file;
    const char*      title;
    eco::log::entry  entry;

    inline message(
        eco::log::level level,
        int line,
        const char* file, 
        const char* title)
        : level(level)
        , time(eco::datetime::now())
        , thread(eco::this_thread::name())
        , tid(eco::this_thread::sid())
        , when(true)
        , line(line)
        , file(file)
        , title(title)
    {}
};


////////////////////////////////////////////////////////////////////////////////
class logger : public eco::rtti::object
{
    eco_rtti_interface(logger, eco::rtti::object);
public:
    struct config
    {
        eco::bool_t     enable = true;
        eco::log::level level_min = eco::log::none;
        eco::log::level level_set = eco::log::none;
        const char*     type = nullptr;
        eco::string     name;
        eco::string     format;
    };

    virtual ~logger() {}

    virtual void on_entry_format(
        eco::log::message& message,
        eco::log::on_time on) = 0;

    virtual void on_entry_output(
        eco::log::message& message) = 0;

protected:
    eco::log::logger::config m_conf;
    friend class elog;
};


////////////////////////////////////////////////////////////////////////////////
class config
{
public:
    eco::bool_t     async;
    eco::log::level level_min;
    eco::log::level level_set;
    uint32_t        cache_size;
    uint32_t        entry_size;
    const char*     format_logger_type;

    inline config()
        : async(false)
        , level_min(eco::log::info)
        , level_set(eco::log::none)
        , cache_size(1 * 1024 * 1024)
        , entry_size(1024)
        , format_logger_type("eco_logging")
    {}
};


////////////////////////////////////////////////////////////////////////////////
class eco_api elog
{
public:
    static void logger(const eco::log::logger::config& conf);

    static void start(const eco::log::config& conf);

    static void format(eco::log::message& message, eco::log::on_time on);

    static void output(eco::log::message& message);

    static eco::bool_t level_check(eco::log::level l);

public:
    static void logger_enable(const char* name, eco::bool_t enable);
    static void logger_level_min(const char* name, eco::log::level level);
    static void logger_level_set(const char* name, eco::log::level level);
};


////////////////////////////////////////////////////////////////////////////////
class stream
    : public eco::stream_t<eco::log::stream>
    , public eco::format_t<eco::log::stream>
{
public:
    inline stream& append(char c, uint32_t size)
    {
        m_message.entry.append(c, size);
        return *this;
    }

    inline stream& append(const char* str, uint32_t size)
    {
        m_message.entry.append(str, size);
        return *this;
    }

public:
    inline stream(
        eco::log::level level, 
        int line,
        const char* file,
        const char* title)
        : m_message(level, line, file, title)
    {
        // to call "logger::on_entry_format(msg, eco::log::on_begin)"
        eco::log::elog::format(m_message, eco::log::on_begin);
    }

    inline ~stream()
    {
        // to call "logger::on_entry_format(msg, eco::log::on_end)"
        // to call "logger::on_entry_output(msg)"
        eco::log::elog::output(m_message);
    }

    inline stream& format(const char* format)
    {
        eco::format_t<eco::log::stream>::reset(format);
        return *this;
    }

    inline stream& printf(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        m_message.entry.printf(format, &args);
        va_end(args);
        return *this;
    }
    
    inline stream& title(const char* title)
    {
        if (m_message.title == NULL)
        {
            m_message.title = title;
            eco::log::elog::format(m_message, eco::log::on_title);
        }
        return *this;
    }

private:
    eco::log::message  m_message;
};



////////////////////////////////////////////////////////////////////////////////
template<uint32_t intv_count, uint32_t intv_duration>
class each
{
public:
    inline each()
    {
        m_time_start = eco::datetime::now();
        m_time_last = m_time_start;
    }

    inline eco::bool_t is()
    {
        // match with "intv_count" or "intv_duration"
        //int64_t curr = eco::datetime::now();
        eco::bool_t cond = (++m_count % intv_count == 0);
        //cond = cond || (curr - m_time_last > intv_duration);
        //if (cond) { m_time_last = curr; }
        return cond;
    }

    inline uint64_t count() const
    {
        return m_count;
    }

    /*inline int64_t start() const
    {
        return m_time_start;
    }*/

private:
    uint64_t m_count = 0;
    eco::datetime m_time_start;
    eco::datetime m_time_last;
};


////////////////////////////////////////////////////////////////////////////////
// eco_log_title
#define eco_log_title_2(title, level) \
for (int w = eco::log::elog::level_check(level); w; w = 0) \
    eco::log::stream(level, __LINE__, __FILE__, title)
#define eco_log_title_3(title, level, when) \
for (int w = (when) && eco::log::elog::level_check(level); w; w = 0) \
    eco::log::stream(level, __LINE__, __FILE__, title)
#define eco_log_title_4(title, level, each_count, each_duration) \
for (int w = eco::log::elog::level_check(level); w; w = 0) \
    for (static const eco::log::each<each_count, each_duration> each; w && \
         const_cast<eco::log::each<each_count, each_duration>&>(each).is(); \
         w = 0) \
        eco::log::stream(level, __LINE__, __FILE__, title)
#define eco_log_title_5(title, level, when, each_count, each_duration) \
for (int w = (when) && eco::log::elog::level_check(level); w; w = 0) \
    for (static const eco::log::each<each_count, each_duration> each; w && \
         const_cast<eco::log::each<each_count, each_duration>&>(each).is(); \
         w = 0) \
        eco::log::stream(level, __LINE__, __FILE__, title)
#define eco_log_title(...) eco_macro_overload(eco_log_title_,__VA_ARGS__)

// eco_log_level
#define eco_log_level_1(level) \
eco_log_title_2(NULL, level)
#define eco_log_level_2(level, when) \
eco_log_title_3(NULL, level, when)
#define eco_log_level_3(level, each_count, each_duration) \
eco_log_title_4(NULL, level, each_count, each_duration)
#define eco_log_level_4(level, when, each_count, each_duration) \
eco_log_title_5(NULL, level, when, each_count, each_duration)
#define eco_log_level(...) eco_macro_overload(eco_log_level_,__VA_ARGS__)

// eco_log
#define eco_log_1(level) \
eco_log_title_2(NULL, eco::log::level)
#define eco_log_2(level, when) \
eco_log_title_3(NULL, eco::log::level, when)
#define eco_log_3(level, each_count, each_duration) \
eco_log_title_4(NULL, eco::log::level, each_count, each_duration)
#define eco_log_4(level, when, each_count, each_duration) \
eco_log_title_5(NULL, eco::log::level, when, each_count, each_duration)
#define eco_log(...) eco_macro_overload(eco_log_,__VA_ARGS__)
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);