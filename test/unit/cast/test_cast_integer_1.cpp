#include <gtest/gtest.h>
#include <eco/cast/cast_integer.hpp>


////////////////////////////////////////////////////////////////////////////////
class cast_integer_from : public ::testing::Test
{
public:



public:
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(cast_integer_from, error)
{
    eco::cast_detail::result r;
    return;

    // 0
    eco::integer_to_string().cast(0, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_TRUE(strcmp(r.c_str(), "0") == 0);
    // int32_t
    eco::integer_to_string().cast(INT32_MAX, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_TRUE(strcmp(r.c_str(), "2147483647") == 0);
    eco::integer_to_string().cast(INT32_MIN, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_TRUE(strcmp(r.c_str(), "-2147483648") == 0);
    // uint32_t
    eco::integer_to_string().cast(UINT32_MAX, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_TRUE(strcmp(r.c_str(), "4294967295") == 0);
    // int64_t
    eco::integer_to_string().cast(INT64_MAX, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_TRUE(strcmp(r.c_str(), "9223372036854775807") == 0);
    eco::integer_to_string().cast(INT64_MIN, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_TRUE(strcmp(r.c_str(), "-9223372036854775808") == 0);
    // uint64_t
    eco::integer_to_string().cast(UINT64_MAX, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_TRUE(strcmp(r.c_str(), "18446744073709551615") == 0);

    // format: base + width + hold + positive
    eco::integer_to_string().format(10, 8, ' ').cast(-12345, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_TRUE(strcmp(r.c_str(), "  -12345") == 0);
    eco::integer_to_string().format(10, 8, '0').cast(12345, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_TRUE(strcmp(r.c_str(), "00012345") == 0);
    eco::integer_to_string().format(10, 8, '-').cast(12345, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_TRUE(strcmp(r.c_str(), "---12345") == 0);
    // format: upper
    eco::integer_to_string().format(16, 0, ' ').upper(1).cast(0x123abc, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_TRUE(strcmp(r.c_str(), "0x123abc") == 0);
    eco::integer_to_string().format(16, 10, ' ').upper(1).cast(0x123abc, r);
    EXPECT_TRUE(!r.fail());
    EXPECT_TRUE(strcmp(r.c_str(), "  0x123ABC") == 0);
}
