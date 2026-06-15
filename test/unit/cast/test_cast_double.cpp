#include <gtest/gtest.h>
#include <cstdlib>
#include <cstring>
////////////////////////////////////////////////////////////////////////////////
#include "ryu_64.h"
#include <eco/cast/cast_double.hpp>


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


////////////////////////////////////////////////////////////////////////////////
// In-house Ryu (eco::double_to_string) behaviour & round-trip tests.
TEST_F(test_cast_double, eco_ryu_basic)
{
    eco::double_to_string<double> conv;

    EXPECT_STREQ(conv.cast(0.0).c_str(),    "0E0");
    EXPECT_STREQ(conv.cast(-0.0).c_str(),   "-0E0");
    EXPECT_STREQ(conv.cast(1.0).c_str(),    "1E0");
    EXPECT_STREQ(conv.cast(-1.0).c_str(),   "-1E0");
    EXPECT_STREQ(conv.cast(0.5).c_str(),    "5E-1");
    EXPECT_STREQ(conv.cast(1.5).c_str(),    "1.5E0");
    EXPECT_STREQ(conv.cast(0.1).c_str(),    "1E-1");
    EXPECT_STREQ(conv.cast(123.456).c_str(),"1.23456E2");
    EXPECT_STREQ(conv.cast(1e10).c_str(),   "1E10");
    EXPECT_STREQ(conv.cast(1e-10).c_str(),  "1E-10");
    EXPECT_STREQ(conv.cast(1.0/0.0).c_str(),  "Infinity");
    EXPECT_STREQ(conv.cast(-1.0/0.0).c_str(), "-Infinity");
}

TEST_F(test_cast_double, eco_ryu_extremes)
{
    char b[40];
    eco::double_to_string<double>::cast(2.2250738585072014e-308, b);
    EXPECT_STREQ(b, "2.2250738585072014E-308");

    eco::double_to_string<double>::cast(1.7976931348623157e+308, b);
    EXPECT_STREQ(b, "1.7976931348623157E308");

    eco::double_to_string<double>::cast(5e-324, b);   // smallest positive subnormal
    EXPECT_STREQ(b, "5E-324");
}

TEST_F(test_cast_double, eco_ryu_round_trip)
{
    // Stress: 50K random doubles must all round-trip through Ryu.
    std::srand(20240904);
    char buf[40];
    int  fails = 0;
    for (int i = 0; i < 50000; ++i)
    {
        uint64_t bits =
            ((uint64_t)std::rand() & 0xFFFF)
            | (((uint64_t)std::rand() & 0xFFFF) << 16)
            | (((uint64_t)std::rand() & 0xFFFF) << 32)
            | (((uint64_t)std::rand() & 0xFFFF) << 48);
        double d;  std::memcpy(&d, &bits, sizeof(d));
        if (d != d) continue;                    // skip NaN
        eco::double_to_string<double>::cast(d, buf);
        double back = std::strtod(buf, nullptr);
        if (back != d) ++fails;
    }
    EXPECT_EQ(fails, 0);
}