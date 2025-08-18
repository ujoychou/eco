#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include "plugin_type.hpp"


////////////////////////////////////////////////////////////////////////////////
class test_plugin : public ::testing::Test
{
public:
    // 
    
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(test_plugin, create)
{
    eco::plugin::path_add("test/unit/plugin");
    database::ptr db;
    // 有配置的情况下
    db = eco::plugin::find<database>("account");
    db = eco::plugin::find<database>("config");
    // 无配置的情况下
    eco::plugin::config conf;
    db = eco::plugin::get<database>("account", "eco_mysql", conf);
    db = eco::plugin::get<database>("market", "eco_pgsql", conf);
}