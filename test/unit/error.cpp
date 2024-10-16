#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/error.hpp>


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
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(ErrorTest, return_error)
{
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