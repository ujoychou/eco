#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include "ryu_64.h"


////////////////////////////////////////////////////////////////////////////////
class test_cast_double : public ::testing::Test
{

};


////////////////////////////////////////////////////////////////////////////////
TEST_F(test_cast_double, ryu)
{
    char buff[64] = {0};
    //ryu::Dtoa(buff, double(314.15));
    //EXPECT_STREQ(buff, "314.15");
    //ryu::Dtoa(buff, double(3.1415926e-100));
    //EXPECT_STREQ(buff, "3.1415926e-100");
    ryu::Dtoa(buff, double(3.1415926e100));
    EXPECT_STREQ(buff, "3.1415926e+100");
}