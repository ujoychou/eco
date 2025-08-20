#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/plugin.hpp>
#include <eco/log.hpp>
#include "plugin_type.hpp"


////////////////////////////////////////////////////////////////////////////////
class test_plugin : public ::testing::Test
{
public:
    // 
    
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(test_plugin, logger_plugin)
{
    eco::log::logger* metric;   // logging performace metric.
    eco::log::logger* blogic;   // logging business logic info.

    // create logger by eco::plugin
    metric = eco::rtti::plugin::get<eco::log::logger>("glog", "1.3.2", "metric");
    ASSERT_TRUE(metric);
    blogic = eco::rtti::plugin::get<eco::log::logger>("blog", "2.0.0", "blogic");
    ASSERT_FALSE(blogic);
    blogic = eco::rtti::plugin::get<eco::log::logger>("blog", "2.1.5", "blogic");
    ASSERT_TRUE(blogic);
    //EXPECT_STREQ(metric->plugin()->name(), "glog");
    //EXPECT_STREQ(metric->plugin()->version(), "1.0.0");
    //EXPECT_STREQ(blogic->plugin()->name(), "blog");
    //EXPECT_STREQ(blogic->plugin()->version(), "2.1.0");

    // using logger api
    eco::log::message msg(eco::log::info, __LINE__, __FILE__, "test");
    metric->on_entry_format(msg, eco::log::on_begin);
    EXPECT_STREQ(msg.entry.text(), "[GLOG_1.0.0] <BEGIN>");
    metric->on_entry_format(msg, eco::log::on_end);
    EXPECT_STREQ(msg.entry.text(), "[GLOG_1.0.0] <BEGIN><END>");
    metric->on_entry_output(msg);
    EXPECT_STREQ(msg.entry.text(), "[GLOG_1.0.0] <BEGIN><END> OUTPUT");
    blogic->on_entry_format(msg, eco::log::on_begin);
    EXPECT_STREQ(msg.entry.text(), "[BLOG_2.1.0] <BEGIN>");
    blogic->on_entry_format(msg, eco::log::on_end);
    EXPECT_STREQ(msg.entry.text(), "[BLOG_2.1.0] <BEGIN><END>");
    blogic->on_entry_output(msg);
    EXPECT_STREQ(msg.entry.text(), "[BLOG_2.1.0] <BEGIN><END> OUTPUT");
}