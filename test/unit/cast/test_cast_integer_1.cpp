#include <gtest/gtest.h>
#include <eco/cast/cast_integer.hpp>


////////////////////////////////////////////////////////////////////////////////
class cast_integer_1 : public ::testing::Test{};


////////////////////////////////////////////////////////////////////////////////
TEST_F(cast_integer_1, error)
{
    eco::cast_detail::result r;
    // 0
    eco::integer_to_string().cast(0, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_STREQ(r.c_str(), "0");
    // int32_t
    eco::integer_to_string().cast(INT32_MAX, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_STREQ(r.c_str(), "2147483647");
    eco::integer_to_string().cast(INT32_MIN, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_STREQ(r.c_str(), "-2147483648");
    // uint32_t
    eco::integer_to_string().cast(UINT32_MAX, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_STREQ(r.c_str(), "4294967295");
    // int64_t
    eco::integer_to_string().cast(INT64_MAX, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_STREQ(r.c_str(), "9223372036854775807");
    eco::integer_to_string().cast(INT64_MIN, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_STREQ(r.c_str(), "-9223372036854775808");
    // uint64_t
    eco::integer_to_string().cast(UINT64_MAX, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_STREQ(r.c_str(), "18446744073709551615");

    // format: base + width + hold + positive
    eco::integer_to_string().format(10, 8, ' ').cast(-12345, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_STREQ(r.c_str(), "  -12345");
    eco::integer_to_string().format(10, 8, '0').cast(12345, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_STREQ(r.c_str(), "00012345");
    eco::integer_to_string().format(10, 8, '-').cast(12345, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_STREQ(r.c_str(), "---12345");
    // format: upper
    eco::integer_to_string().format(16, 0, ' ').cast(0x123abc, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_STREQ(r.c_str(), "0X123ABC");
    eco::integer_to_string().format(16, 10, ' ').lower(1).cast(0x123abc, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_STREQ(r.c_str(), "  0x123abc");
}
