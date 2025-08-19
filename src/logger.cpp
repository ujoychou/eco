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
    // time [thread] [level] <title> message (file:line)
    // 20250813 12:59:56.405324 [event] [ info] <dds-someip> load mapping...(file.c:161)
    static inline void format_title(eco::log::message& msg)
    {
        eco::input<eco::log::entry> input(msg.entry);
        if (msg.title != nullptr) { input << eco::square(msg.title); }
    }
    static inline void format_begin(eco::log::message& msg)
    {
        eco::input<eco::log::entry> input(msg.entry);
        input << msg.time.stamp(eco::datetime::iso_m) << ' ';
        input << eco::square(eco::empty(msg.thread) ? msg.tid : msg.thread);
        input << ' ' << eco::square(eco::log::name(msg.level)) << ' ';
        format_title(msg);
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

    static inline void format_message(
        eco::log::message& msg, const eco::string& format, eco::log::on_time on)
    {
        if (on == on_begin)
        {
            if (format.empty())
            {
                logger_logging::format_begin(msg);
            }
        }
        else if (on == on_end)
        {
            logger_logging::format_end(msg);
        }
        else if (on == on_title)
        {
            logger_logging::format_title(msg);
        }
    }

public:
    virtual void on_entry_format(eco::log::message& msg, on_time on) override
    {
        format_message(msg, m_conf.format, on);
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
    virtual void on_entry_format(eco::log::message& msg, on_time on) override
    {
        logger_logging::format_message(msg, m_conf.format, on);
    }

    virtual void on_entry_output(eco::log::message& msg) override
    {
        std::cout << msg.entry.text() << std::endl;
    }
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);