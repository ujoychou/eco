#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/log.hpp>


////////////////////////////////////////////////////////////////////////////////
class LogTest : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
    }
    static void TearDownTestSuite()
    {
    }

    // parse log result as a structure
    inline std::string& result();
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(LogTest, factory)
{
    eco::log::config conf;
    eco::log::logger::ptr obj = eco::log::create(conf);
    obj->start(&conf);
    obj->stop();
    
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(LogTest, level)
{
    eco_fatal();
    ASSERT_EQ();
    eco_error();
    eco_warn();
    eco_info();
    eco_debug();
    eco_log(eco::log::l9, 0);
    eco_log(eco::log::l8, 0);
    eco_log(eco::log::l7, 0);
    eco_log(eco::log::l6, 0);
    eco_log(eco::log::l5, 0);
    eco_log(eco::log::l4, 0);
    eco_log(eco::log::l3, 0);
    eco_log(eco::log::l2, 0);
    eco_log(eco::log::l1, 0);
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(LogTest, level)
{
    eco_fatal();
    //ASSERT_EQ();
    eco_error();
    eco_warn();
    eco_info();
    eco_debug();
    eco_log(eco::log::l9, 0);
    eco_log(eco::log::l8, 0);
    eco_log(eco::log::l7, 0);
    eco_log(eco::log::l6, 0);
    eco_log(eco::log::l5, 0);
    eco_log(eco::log::l4, 0);
    eco_log(eco::log::l3, 0);
    eco_log(eco::log::l2, 0);
    eco_log(eco::log::l1, 0);
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(LogTest, format_error)
{
    eco_error();
    //const char* url = "10.21.153.55:9050/images/icon/cool.jpg";
    //eco::error("login fail, username or password is error.");
     //eco::error("get file error: %1 don't have %2 privilege of file %3") % "zhouyu" % 1 % url;
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(LogTest, format_error)
{
    eco_error();
    //const char* url = "10.21.153.55:9050/images/icon/cool.jpg";
    //eco::error("login fail, username or password is error.");
     //eco::error("get file error: %1 don't have %2 privilege of file %3") % "zhouyu" % 1 % url;
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(LogTest, throw_error)
{
}