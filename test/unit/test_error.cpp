#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/error.hpp>
#include <limits>


////////////////////////////////////////////////////////////////////////////////
class ErrorTest : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
    }
    static void TearDownTestSuite()
    {
    }

    inline bool return_error_code_login_user(const char* user)
    {
        if (!return_error_check_user(user))
        {
            return eco_trace();
        }
    }

    inline bool return_error_code_check_user(const char* user)
    {
        eco::trace(1001) << user;
        return false;
    }
};

////////////////////////////////////////////////////////////////////////////////
TEST_F(ErrorTest, format_error)
{
    eco::error(1001);
    eco::error(1001, 
    "format: dmax=%d/dmin=%d umax=%u/umin=%u fmax=%f/fmin=%f dmax=%g/dmin=%g "
    "string=%s",
    std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min(),
    std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::min(),
    std::numeric_limits<float>::max(), std::numeric_limits<float>::min(),
    std::numeric_limits<double>::max(), std::numeric_limits<double>::min(),
    "string_test_format_error");
    ASSERT_EQ(!eco::error().message().empty(), true);
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(ErrorTest, return_error)
{
    if (!user_login())
    {
        eco_trace();
        return false;      
    }
    
    //const char* url = "10.21.153.55:9050/images/icon/cool.jpg";
    //eco::error("login fail, username or password is error.");
     //eco::error("get file error: %1 don't have %2 privilege of file %3") % "zhouyu" % 1 % url;
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(ErrorTest, throw_error)
{
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(ErrorTest, multiple_language)
{
    /* <en_us/error.json>
    {
        "trade": {
            "subaccount": {
                "401": "The sub-account %1 does not exist, which hasn't been registered."
                "402": "The balance of sub-account %1 is insufficient. The current balance is %2, which is less than the %3 required for this transaction."
            }
        }
    }
    */


    try
    {
        eco_throw("/trade/subaccount/401") % "S90092" % 1000 % 1300;
    }
    catch(const eco::error& e)
    {
        ASSERT_TRUE(e.message() == "The balance of sub-account S90092 is "
"insufficient. The current balance is 1000, which is less than the 1300 "
"required for this transaction.");
    }
}