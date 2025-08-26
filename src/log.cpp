#include <eco/log.hpp>
////////////////////////////////////////////////////////////////////////////////
#include <memory>
#include <vector>
#include "log_impl.hpp"


eco_namespace(eco);
eco_namespace(log);
////////////////////////////////////////////////////////////////////////////////
struct elog_impl
{
public:
    typedef std::shared_ptr<eco::log::logger> logger_ptr;
    eco::log::level         level_min = eco::log::none;
    eco::log::level         level_set = eco::log::none;
    logger_ptr              format;
    eco::log::config        config;
    std::vector<logger_ptr> loggers;

    inline elog_impl() {}
};


////////////////////////////////////////////////////////////////////////////////
static eco::log::elog_impl g_impl;
void elog::logger(const eco::log::logger::config& conf)
{
    elog_impl::logger_ptr logger = eco::log::logger::create(conf.type);
    logger->m_conf = conf;
    g_impl.loggers.push_back(logger);
}


////////////////////////////////////////////////////////////////////////////////
void elog::start(const eco::log::config& conf)
{
    // entry cache
    uint32_t size = eco::align_up(conf.entry_size, 8);
    uint32_t count = conf.cache_size / size;
    eco::cache::init_logs(size, count);
    g_impl.config = conf;

    // format logger
    for (elog_impl::logger_ptr& logger : g_impl.loggers)
    {
        if (logger->same_of(conf.format_logger_type))
        {
            g_impl.format = logger;
        }
    }
    if (!g_impl.format)
    {
        g_impl.format = eco::log::logger::create(conf.format_logger_type);
    }

    // logger level
    g_impl.level_min = conf.level_min;
    g_impl.level_set = conf.level_set;
    for (elog_impl::logger_ptr& logger : g_impl.loggers)
    {
        // level_min
        if (logger->m_conf.level_min == eco::log::none)
            logger->m_conf.level_min = conf.level_min;
        else if (g_impl.level_min > logger->m_conf.level_min)
            g_impl.level_min = logger->m_conf.level_min;
        // level_set
        if (logger->m_conf.level_set == eco::log::none)
            logger->m_conf.level_set = conf.level_set;
        else
            g_impl.level_set |= logger->m_conf.level_set;
    }
}


////////////////////////////////////////////////////////////////////////////////
void elog::format(eco::log::message& msg, eco::log::on_time on)
{
    if (msg.entry.text() == nullptr)
    {
        msg.entry = eco::cache::entry_this_borrow();
    }
    g_impl.format->on_entry_format(msg, on);
}

void elog::output(eco::log::message& msg)
{
    g_impl.format->on_entry_format(msg, eco::log::on_end);
    for (elog_impl::logger_ptr& logger : g_impl.loggers)
    {
        logger->on_entry_output(msg);
    }
    eco::cache::entry_this_return(msg.entry);
}


////////////////////////////////////////////////////////////////////////////////
eco::bool_t elog::level_check(eco::log::level l)
{
    return (l >= g_impl.level_min) || (l & g_impl.level_set);
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);