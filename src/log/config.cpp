/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2025-08-13] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/log/config.hpp>
#include <eco/string.hpp>
#include <eco/export/impl.hpp>


eco_namespace(eco);
eco_namespace(log);
////////////////////////////////////////////////////////////////////////////////
class config_sink::impl
{
public:
    int level = 0;
    uint16_t modula_level[32] = { 1 };
    uint32_t roll_size = 0;
    std::string name;
    std::string path;
    std::string file;
};
eco_shared_impl(config_sink, config_sink::impl);
eco_member_impl_string(config_sink, name);
eco_member_impl_string(config_sink, path);
eco_member_impl_string(config_sink, file);
eco_member_impl_value(config_sink, int, level);
eco_member_impl_value(config_sink, uint32_t, roll_size);
int config_sink::level(int modula) const
{
    return m_impl->modula_level[modula];
}

config_sink& config_sink::level(int modula, int value)
{
    m_impl->modula_level[modula] = value;
    return *this;
}


////////////////////////////////////////////////////////////////////////////////
class config::impl
{
public:
    static const int sink_max_count = 8;

    uint32_t cache_size = 0;
    uint32_t entry_size = 0;
    eco::bool_t async = false;
    eco::bool_t aspect = false;
    std::string format;
    std::string modula;
    config_sink sinks[sink_max_count];
};
eco_shared_impl(config, config::impl);
eco_member_impl_value(config, uint32_t, cache_size);
eco_member_impl_value(config, uint32_t, entry_size);
eco_member_impl_value(config, eco::bool_t, async);
eco_member_impl_value(config, eco::bool_t, aspect);
eco_member_impl_string(config, format);
eco_member_impl_string(config, modula);
config_sink& config::sink(const char* name)
{
    int i = 0;
    for (; i < impl::sink_max_count; i++)
    {
        config_sink& sink = m_impl->sinks[i];
        if (sink.null()) { break; }
        if (eco::equal(sink.name(), name)) { return sink; }
    }
    //eco_throw(1001, i == impl::sink_max_count);
    m_impl->sinks[i] = config_sink(eco::heap);
    m_impl->sinks[i].name(name);
    return m_impl->sinks[i];
}


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);