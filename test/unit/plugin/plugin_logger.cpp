#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/plugin.hpp>


////////////////////////////////////////////////////////////////////////////////
class test_plugin : public ::testing::Test
{
public:
    
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(test_plugin, create_api)
{
    eco::plugin::path_add("test/unit/plugin");
    eco::database::ptr db;
    // 无配置的情况下
    db = eco::plugin::create<eco::database>("eco_mysql");
    eco::plugin::add("account", db);
    db = eco::plugin::create<eco::database>("eco_sqlite");
    eco::plugin::add("config", db);

    // 有配置的情况下
    db = eco::plugin::get<eco::database>("account", "eco_mysql");
    db = eco::plugin::get<eco::database>("config", "eco_sqlite");
    // 等同于
    eco::database::ptr silkway = eco::app().persist("silkway");
}


////////////////////////////////////////////////////////////////////////////////
class mysql : public eco::database
{
    eco_plugin("eco_mysql");
public:
    
};


////////////////////////////////////////////////////////////////////////////////
class sqlite : public eco::database
{
    eco_plugin("eco_sqlite");
public:
};


////////////////////////////////////////////////////////////////////////////////
class pgsql : public eco::database
{
    eco_plugin("eco_pgsql");
public:
};


void create()
{
    const char* type = dll.type();
    const char* 
}


////////////////////////////////////////////////////////////////////////////////
void main()
{
    auto x = eco::plugin::load("/a/b/c.dll");
    x->name();
    x->create();

}