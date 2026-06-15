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
#include <eco/export/export.hpp>


eco_namespace(eco);
eco_namespace(log);
////////////////////////////////////////////////////////////////////////////////
class eco_api config_sink
{
    eco_shared_api(config_sink);
public:
    const char*  name() const;
    config_sink& name(const char* value);

    int level() const;
    config_sink& level(int value);
    
    int level(int modula) const;
    config_sink& level(int modula, int value);
    
    const char* path() const;
    config_sink& path(const char* value);

    const char* file() const;
    config_sink& file(const char* value);

    uint32_t roll_size() const;
    config_sink& roll_size(uint32_t value);
};


////////////////////////////////////////////////////////////////////////////////
class eco_api config
{
    eco_shared_api(config);
public:
    uint32_t cache_size() const;
    config& cache_size(uint32_t size);

    uint32_t entry_size() const;
    config& entry_size(uint32_t size);

    eco::bool_t async() const;
    config& async(eco::bool_t v);

    const char* modula() const;
    config& modula(const char* name);

    eco::bool_t aspect() const;
    config& aspect(eco::bool_t v);
    
    const char* format() const;
    config& format(const char* format);
    
    config_sink& sink(const char* name);
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);