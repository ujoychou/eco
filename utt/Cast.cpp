#include <gtest/gtest.h> 
////////////////////////////////////////////////////////////////////////////////
#include <eco/Cast.h>


////////////////////////////////////////////////////////////////////////////////
TEST(BaseType, CastBool)
{
    // boolean
    EXPECT_EQ(eco::cast<bool>(nullptr), false);
    EXPECT_EQ(eco::cast<bool>(""), false);
    EXPECT_EQ(eco::cast<bool>("false"), false);
    EXPECT_EQ(eco::cast<bool>("True"), true);
    EXPECT_EQ(eco::cast<bool>("true"), true);
    EXPECT_EQ(eco::cast<bool>("0"), false);
    EXPECT_EQ(eco::cast<bool>("2"), false);
    EXPECT_EQ(eco::cast<bool>("1"), true);
    EXPECT_EQ(eco::cast(true),  "1");
    EXPECT_EQ(eco::cast(false), "0");
    // char
    EXPECT_EQ(eco::cast<char>("char"), 'c');
    EXPECT_EQ(eco::cast<char>("0101"), '0');
    EXPECT_EQ(eco::cast<unsigned char>("char"), 'c');
    EXPECT_EQ(eco::cast<unsigned char>("0101"), '0');
    //EXPECT_EQ(eco::cast('c'), "c");
    //EXPECT_EQ(eco::cast('0'), "0");
    //using std::string val(1, 'c');
}


////////////////////////////////////////////////////////////////////////////////
TEST(BaseType, CastInteger)
{
    // int16_t
    EXPECT_EQ(eco::cast(int16_t(0)), "0");
    EXPECT_EQ(eco::cast(int16_t(23445)), "23445");
    EXPECT_EQ(eco::cast(int16_t(-1111)), "-1111");
    EXPECT_EQ(eco::cast<int16_t>("0"), 0);
    EXPECT_EQ(eco::cast<int16_t>("12345"), 12345);
    EXPECT_EQ(eco::cast<int16_t>("-1111"), -1111);
    EXPECT_EQ(eco::cast<int16_t>("12345", 3), 123);
    EXPECT_EQ(eco::cast<int16_t>("-1111", 3), -11);
    EXPECT_EQ(eco::cast(uint16_t(65535)), "65535");
    EXPECT_NE(eco::cast(uint16_t(-3444)), "-3444");
    EXPECT_EQ(eco::cast(uint16_t(65536)), "0");
    EXPECT_EQ(eco::cast<uint16_t>("65535"), 65535);
    EXPECT_EQ(eco::cast<uint16_t>("-1111"), uint16_t(-1111));
    EXPECT_EQ(eco::cast<uint16_t>("65536"), 0);
    EXPECT_EQ(eco::cast<uint16_t>("34562"), 34562);
    EXPECT_EQ(eco::cast<uint16_t>("-12345"), uint16_t(-12345));
    EXPECT_EQ(eco::cast<uint16_t>("34562", 3), 345);
    EXPECT_EQ(eco::cast<uint16_t>("-12345", 5), uint16_t(-1234));
    EXPECT_EQ(eco::cast<uint16_t>("-12345", 1), 0);

    // uint32_t
    EXPECT_EQ(eco::cast(1234567890), "1234567890");
    EXPECT_EQ(eco::cast(-23456789), "-23456789");
    EXPECT_EQ(eco::cast<int32_t>("65536"), 65536);
    EXPECT_EQ(eco::cast<int32_t>("-23456789"), -23456789);
    EXPECT_EQ(eco::cast<int32_t>("-10456789"), -10456789);
    EXPECT_EQ(eco::cast<int32_t>("-12345"), -12345);
    EXPECT_EQ(eco::cast<int32_t>("2345678910", 8), 23456789);
    EXPECT_EQ(eco::cast<int32_t>("-2345678910", 6), -23456);
    EXPECT_NE(eco::cast(uint32_t(-13456789)), "-13456789");
    EXPECT_EQ(eco::cast<uint32_t>("1234567890"), uint32_t(1234567890));
    EXPECT_EQ(eco::cast<uint32_t>("2345678901"), uint32_t(2345678901));
    EXPECT_EQ(eco::cast<uint32_t>("-12345"), uint32_t(-12345));
    EXPECT_EQ(eco::cast<uint32_t>("2345678910", 8), uint32_t(23456789));
    EXPECT_EQ(eco::cast<uint32_t>("-2345678910", 6), uint32_t(-23456));
    // uint64
    EXPECT_EQ(eco::cast(1234567890123456), "1234567890123456");
    EXPECT_EQ(eco::cast(-1234567890123456), "-1234567890123456");
}