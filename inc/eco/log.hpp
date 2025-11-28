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
#include <eco/log/message.hpp>
#include <eco/log/config.hpp>
#include <eco/log/each.hpp>
#include <stdarg.h>


eco_namespace(eco);
eco_namespace(log);
////////////////////////////////////////////////////////////////////////////////
class eco_api elog
{
public:
    class impl;
    static void start(const eco::log::config& conf);
    static void modula(int id, const char* name);
    static eco::bool_t level_match(int level);
    static void format(eco::log::message& msg);
    static void output(eco::log::message& msg);
};


////////////////////////////////////////////////////////////////////////////////
class stream
    : public eco::stream_t<eco::log::stream>
    , public eco::format_t<eco::log::stream>
{
private:
    inline void message_head_format()
    {
        if (m_message.entry.null())
        {
            eco::log::elog::format(m_message);
        }
    }

    inline eco::string_entry& entry()
    {
        message_head_format();
        return m_message.entry;
    }

public:
    inline stream& append(char c, uint32_t size)
    {
        entry().append(c, size);
        return *this;
    }

    inline stream& append(const char* str, uint32_t size)
    {
        entry().append(str, size);
        return *this;
    }

public:
    inline stream(int level, int line, const char* file, const char* func)
        : m_message(level, line, file, func)
    {}

    inline stream& aspect(const char* v)
    {
        m_message.aspect = v;
        return *this;
    }

    inline stream& mode(int v)
    {
        m_message.level |= (v << 24);
        return *this;
    }

    inline stream& user(const char* v)
    {
        m_message.user = v;
        return *this;
    }

    inline ~stream()
    {
        eco::log::elog::output(m_message);
    }

    inline stream& format(const char* format, ...)
    {
        //message_head_format();
        //eco::format_t<eco::log::stream>::format(format);
        return *this;
    }

    inline stream& printf(const char* format, ...)
    {
        //message_head_format();
        //eco::format_t<eco::log::stream>::printf(format, args);
        return *this;
    }

private:
    eco::log::message m_message;
};


////////////////////////////////////////////////////////////////////////////////
struct modula_register
{
    inline modula_register(int modula, const char* name)
    {
        eco::log::elog::modula(modula, name);
    }
};

template<int modula, const char* name>
struct modula_init
{
    static modula_register reg;
    inline static int l(int level) { return level; }
};
template<int modula, const char* name>
eco::log::modula_register modula_init<modula, name>::reg(modula, name);
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);


////////////////////////////////////////////////////////////////////////////////
// eco_log_level
#define eco_log_level_1(level) \
for (int w = eco::log::elog::level_match(level); w; w = 0) \
    eco::log::stream(level, __LINE__, __FILE__, __func__)
#define eco_log_level_2(level, when) \
for (int w = (when) && eco::log::elog::level_match(level); w; w = 0) \
    eco::log::stream(level, __LINE__, __FILE__, __func__)
#define eco_log_level_3(level, each_count, each_duration) \
for (int w = eco::log::elog::level_match(level); w; w = 0) \
    for (static const eco::log::each<each_count, each_duration> each; \
         w && const_cast<eco::log::each<each_count, each_duration>&>(each).is(); \
         w = 0) \
        eco::log::stream(level, __LINE__, __FILE__, __func__)
#define eco_log_level_4(level, when, each_count, each_duration) \
for (int w = (when) && eco::log::elog::level_match(level); w; w = 0) \
    for (static const eco::log::each<each_count, each_duration> each; \
         w && const_cast<eco::log::each<each_count, each_duration>&>(each).is(); \
         w = 0) \
        eco::log::stream(level, __LINE__, __FILE__, __func__)
#define eco_log_level(...) eco_macro_overload(eco_log_level_,__VA_ARGS__)

// eco_log
#define eco_log_1(level) \
eco_log_level_1(eco::log::level)
#define eco_log_2(level, when) \
eco_log_level_2(eco::log::level, when)
#define eco_log_3(level, each_count, each_duration) \
eco_log_level_3(eco::log::level, each_count, each_duration)
#define eco_log_4(level, when, each_count, each_duration) \
eco_log_level_4(eco::log::level, when, each_count, each_duration)
#define eco_log(...) eco_macro_overload(eco_log_,__VA_ARGS__)

// eco_log_modula
#define eco_log_modula(modula, modula_name, level, ...) \
eco_log_level(modula_init<modula, modula_name>::l(eco::log::level), __VA_ARGS__)
// exp: modula_log
// #define modula1_log(l, ...) eco_log_modula(1, "modula1", l, __VA_ARGS__)
// #define modula2_log(l, ...) eco_log_modula(2, "modula2", l, __VA_ARGS__)
// exp: aspect_log
// #define aspect1_log(l, ...) eco_log(l, __VA_ARGS__).aspect("aspect1")
// #define aspect2_log(l, ...) eco_log(l, __VA_ARGS__).aspect("aspect2")