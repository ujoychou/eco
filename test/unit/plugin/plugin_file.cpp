////////////////////////////////////////////////////////////////////////////////
#include <eco/plugin.hpp>
#include <eco/plugin/app.hpp>
#include "plugin_type.hpp"


////////////////////////////////////////////////////////////////////////////////
class test_mysql : public file
{
    eco_plugin(test_mysql, "mysql", "1.0.0");
public:
    virtual void open() override
    {
    }

    virtual void close() override
    {
    }
};


////////////////////////////////////////////////////////////////////////////////
class test_sqlite : public file
{
    eco_plugin(test_sqlite, "sqlite", "2.1.0");
public:
    static eco::bool_t compatible_with(const char* version)
    {
        return eco::version_compare(version, "2.1.0") >= 0;
    }

    static eco::bool_t can_open(const char* path)
    {}
};


////////////////////////////////////////////////////////////////////////////////
class FileRxApp : public eco::rx::app
{
public:
    eco::result on_init() override
    {
        eco::plugin::set_plugin<test_mysql>();
        eco::plugin::set_plugin<test_sqlite>();
    }

    eco::result on_exit() override
    {
    }
};


////////////////////////////////////////////////////////////////////////////////
eco_erx(FileRxApp, erx_app);