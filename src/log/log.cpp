#include <eco/log.hpp>
////////////////////////////////////////////////////////////////////////////////
#include <memory>
#include <vector>


eco_namespace(eco);
eco_namespace(log);
////////////////////////////////////////////////////////////////////////////////
class elog::impl
{
public:
    eco::log::config        m_config;
    eco::string_view        m_modula_name[32];
    uint16_t                m_modula_level[32] = { 1 };

public:
    static inline const char* level_name(int l)
    {
        switch (eco::log::level_level(l))
        {
        case eco::log::info:  return "info ";
        case eco::log::warn:  return "warn ";
        case eco::log::error: return "error";
        case eco::log::debug: return "debug";
        case eco::log::trace: return "trace";
        case eco::log::fatal: return "fatal";
        default: break;
        }
        return "none ";
    }

    inline const char* modula_name(int l)
    {
        int m = eco::log::level_modula(l);
        return m_modula_name[m].c_str();
    }

    static inline const char* file_name(const char* name, int nth)
    {
        return "";
    }

    static inline eco::bool_t level_match(int level, int setting)
    {
        return (setting & level) == level;
    }
};
static eco::log::elog::impl& impl()
{
    static eco::log::elog::impl inst;
    return inst;
}


////////////////////////////////////////////////////////////////////////////////
void elog::start(const eco::log::config& conf)
{
    // entry cache
    uint32_t size = eco::align_up(conf.entry_size(), 8);
    uint32_t count = conf.cache_size() / size;
    //eco::cache::init_logs(size, count);
    impl().m_config = conf;
}


// time (thread) [level] (chain) <module.aspect> mode @user message {pos} #error
////////////////////////////////////////////////////////////////////////////////
void format(eco::log::message& msg)
{    
    // "time (thread) [level] (chain)"
    eco::string_entry entry;// = eco::cache::entry_borrow_log();
    //eco::input<eco::log::entry> input(entry);
    //input << msg->time.stamp(eco::datetime::iso_m) << ' ';
    //entry << eco::group(msg->thread) << ' ';
    entry << eco::square(eco::log::level_name(eco::log::level_level(msg.level))) << ' ';
    entry << eco::group(msg->chain) << ' ';

    // <module.aspect>
    const char* modula = impl::modular_name(impl::modula(msg.level));
    if (modula == 0) { modula = impl().modula_name[modula_all].data(); }
    if (modula || msg.aspect)
    {
        input << '<';
        if (modula) { input << modula; }
        if (modula && msg.aspect) { input << '.'; }
        if (msg.aspect) { input << msg.aspect; }
        input << "> ";
    }

    // mode @user
    input << impl().mode_name(msg.mode) << ' ';
    input << '@' << msg.user << ' ';
}


////////////////////////////////////////////////////////////////////////////////
void output(eco::log::message& msg)
{
    eco::input<eco::log::entry> input(msg.entry);

    // {pos} = {file:line}
    if (impl::level_match(msg.level, impl().config.level_with_pos))
    {
        input << '{' << eco::log::filename(msg.file, 2);
        input << ':' << msg.line << '}';
    }
    // #error
    if (msg.error) { input << '#' << msg.error; }

    // async
    if (impl().config.async)
    {
        impl().async_queue.push(msg.entry);
    }
    else
    {
        impl().output(msg.entry);
    }
}


////////////////////////////////////////////////////////////////////////////////
void modula(const char* name)
{
    impl().modula_name[modula_all] = eco::string_view(name);
}
void modula(int id, const char* name)
{
    if (id >= 0 && id <= eco::log::modula_max)
    {
        impl().modula_name[id] = eco::string_view(name);
    }
}


////////////////////////////////////////////////////////////////////////////////
eco::bool_t level_match(eco::log::level l)
{
    int m = eco::log::modula_of_level(l);
    l = eco::log::level_of_level(l);
    return m <= modula_max && impl::level_match(l, impl().modula_level[m]);
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);