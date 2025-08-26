#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/error.hpp>
#include <limits>


////////////////////////////////////////////////////////////////////////////////
class TestError : public ::testing::Test
{
public:
    inline bool user_exist()
    {
        return false;
    }
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(TestError, macro_format)
{
    // eco_error: printf style
    eco_error(1001, "user %s not exist", "zhangsan");
    eco_error("module/E1001", "user %s not exist", "zhangsan");

    bool exist = user_exist();
    eco_error_if(exist, 1002, "user %s not exist", "zhangsan");

    if (!user_exist())
    {
        eco_error(1001, "user %s not exist", "zhangsan");
        return ;
    }
    {
        /* code */
    }
    
}