#include <eco/log.hpp>
////////////////////////////////////////////////////////////////////////////////
#include "log_impl.hpp"
#include <iostream>


eco_namespace(eco);
eco_namespace(log);
//##############################################################################
////////////////////////////////////////////////////////////////////////////////
class logger_logging : public eco::log::logger
{
    eco_rtti(logger_logging, eco::log::logger);
public:
    static inline void format_begin(eco::log::message& msg)
    {
        // time [thread] [level] <title> message (file:line)
        // 20250813 12:59:56.405324 [event] [ INFO] load mapping...(file.c:161)
        eco::input<eco::log::entry> input(msg.entry);
        input << msg.time.stamp(eco::datetime::iso_m) << ' ';
        input << eco::square(eco::empty(msg.thread) ? msg.tid : msg.thread);
        input << ' ' << eco::square(eco::log::name(msg.level)) << ' ';
    }

    static inline void format_begin(message& msg, const eco::string& fmt)
    {
    }

    static inline void format_end(message& msg)
    {
        // (file:line)
        if (msg.level >= eco::log::warn)
        {
            eco::input<eco::log::entry> input(msg.entry);
            input << '(' << eco::log::filename(msg.file, 2);
            input << ':' << msg.line << ')';
        }
    }

public:
    virtual void on_entry_format_begin(eco::log::message& msg) override
    {
        return m_conf.format.empty() 
            ? logger_logging::format_begin(msg)
            : logger_logging::format_begin(msg, m_conf.format);
    }

    virtual void on_entry_format_end(eco::log::message& msg) override
    {
        logger_logging::format_end(msg);
    }

    virtual void on_entry_output(eco::log::message& msg) override
    {
        std::cout << msg.entry.text() << std::endl;
    }
};


//##############################################################################
////////////////////////////////////////////////////////////////////////////////
class logger_console : public eco::log::logger
{
    eco_rtti(logger_console, eco::log::logger);
public:
    virtual void on_entry_format_begin(eco::log::message& msg) override
    {
        return m_conf.format.empty() 
            ? logger_logging::format_begin(msg)
            : logger_logging::format_begin(msg, m_conf.format);
    }

    virtual void on_entry_format_end(eco::log::message& msg) override
    {
        logger_logging::format_end(msg);
    }

    virtual void on_entry_output(eco::log::message& msg) override
    {
        std::cout << msg.entry.text() << std::endl;
    }
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);