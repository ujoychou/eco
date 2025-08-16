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
    eco_rtti_i(logger, eco::rtti::object);
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
    virtual void on_entry_format_begin(eco::log::message& message) = 0;
    virtual void on_entry_format_end(eco::log::message& message) = 0;
    virtual void on_entry_output(eco::log::message& message) = 0;

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

    static void format(eco::log::message& message);

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
        // to call "logger::on_entry_format_begin()"
        eco::log::elog::format(m_message);
    }

    inline ~stream()
    {
        // to call "logger::on_entry_format_end()"
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
    
    template<template<typename q> class wrap_t = eco::angle_t>
    inline stream& title(const char* title)
    {
        if (m_message.title == NULL)
        {
            m_message.title = title;
        }
        (*this) << wrap_t<const char*>(title);
        return *this;
    }

private:
    eco::log::message  m_message;
};


////////////////////////////////////////////////////////////////////////////////
#define eco_log_impl_when(level, when, title) \
for (int w = (when) && eco::log::elog::level_check(level); w; w = 0) \
    eco::log::stream(level, __LINE__, __FILE__, title)
#define eco_log_impl_each(level, when, each_count, each_duration, title) \
for (int w = (when) && eco::log::elog::level_check(level); w; w = 0) \
    for (static eco::log::each each(each_count, each_duration); \
         w && each.is(); w = 0) \
        eco::log::stream(level, __LINE__, __FILE__, title)

// eco_log
#define eco_log_1(level) \
eco_log_impl_when(eco::log::level, 1, NULL)
#define eco_log_2(level, when) \
eco_log_impl_when(eco::log::level, when, NULL)
#define eco_log_3(level, each_count, each_duration) \
eco_log_impl_each(eco::log::level, 1, each_count, each_duration, NULL)
#define eco_log_4(level, when, each_count, each_duration) \
eco_log_impl_each(eco::log::level, when, each_count, each_duration, NULL)
#define eco_log(...) eco_macro_overload(eco_log_,__VA_ARGS__)

// eco_log_level
#define eco_log_level_1(level) \
eco_log_impl_when(level, 1, NULL)
#define eco_log_level_2(level, when) \
eco_log_impl_when(level, when, NULL)
#define eco_log_level_3(level, each_count, each_duration) \
eco_log_impl_each(level, 1, each_count, each_duration, NULL)
#define eco_log_level_4(level, when, each_count, each_duration) \
eco_log_impl_each(level, when, each_count, each_duration, NULL)
#define eco_log_level(...) eco_macro_overload(eco_log_level_,__VA_ARGS__)
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);