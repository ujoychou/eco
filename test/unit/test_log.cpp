#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/log.hpp>
#include <eco/string/string.hpp>


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
    static inline eco::string& result()
    {
        return thread_buff;
    }
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(LogTest, factory)
{   
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(LogTest, format)
{
    eco::log::config& conf = eco::log::logger::get().config();
    conf.level();
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
    eco_log(debug);
    eco_log(warn );
    eco_log(error);
    eco_log(fatal);
    eco_log();
    eco_log(eco::log::l9, 0);
    eco_log(eco::log::l8, 0);
    eco_log(eco::log::l7, 0);
    eco_log(eco::log::l6, 0);
    eco_log(eco::log::l5, 0);
    eco_log(eco::log::l4, 0);
    eco_log(eco::log::l3, 0);
    eco_log(eco::log::l2, 0);
    eco_log(eco::log::l1, 0);

    int a = 0, b = 1;
    eco_log(debug).when(a < b).each(3).f("xxxx%1 xxx%2").p(1).p(3);
    eco_log(debug).when(a < b).each(3).format("xxxx%1 xxx%2") % a % b;

    eco_log(debug, "sdfsdfsdfsdfsdfs", 1, 2.33, "cool").each(3).when(a > 9);
    eco_log(debug, (a > 9), 100, "sdfsdfsdfsdfsdfs", 1, 2.33, "cool");
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(LogTest, format)
{
}
