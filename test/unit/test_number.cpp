/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2025-06-26] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <gtest/gtest.h>
#include <eco/type/number.hpp>


////////////////////////////////////////////////////////////////////////////////
class number : public ::testing::Test
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
TEST_F(number, align)
{
    // count_highest_bit
    ASSERT_EQ(eco::count_highest_bit(0), 0u);
    ASSERT_EQ(eco::count_highest_bit(1), 1u);
    for (uint64_t i = 1; i < 64; i++)
    {
        uint64_t v_min = (1ull << i);
        uint64_t v_max = (1ull << i) - 1;
        ASSERT_EQ(eco::count_highest_bit(v_min), i + 1);
        ASSERT_EQ(eco::count_highest_bit(v_max), i);
    }

    // clzll
    ASSERT_EQ(eco::clzll(0), 64u);
    ASSERT_EQ(eco::clzll(1), 63u);
    for (uint64_t i = 1; i < 64; i++)
    {
        uint64_t v_min = (1ull << i);
        uint64_t v_max = (1ull << i) - 1;
        ASSERT_EQ(eco::clzll(v_min), 63 - i);
        ASSERT_EQ(eco::clzll(v_max), 64 - i);
    }

    // chbll: same with count_highest_bit.
    ASSERT_EQ(eco::chbll(0), 0u);
    ASSERT_EQ(eco::chbll(1), 1u);
    for (uint64_t i = 1; i < 64; i++)
    {
        uint64_t v_min = (1ull << i);
        uint64_t v_max = (1ull << i) - 1;
        ASSERT_EQ(eco::chbll(v_min), i + 1);
        ASSERT_EQ(eco::chbll(v_max), i);
    }

    // power_of_two
    ASSERT_TRUE(eco::power_of_two(0));
    ASSERT_TRUE(eco::power_of_two(1));
    ASSERT_TRUE(eco::power_of_two(2));
    for (uint64_t i = 2; i < 64; i++)
    {
        uint64_t v_min = (1ull << i);
        uint64_t v_max = (1ull << i) - 1;
        ASSERT_TRUE(eco::power_of_two(v_min));
        ASSERT_FALSE(eco::power_of_two(v_max));
        ASSERT_FALSE(eco::power_of_two(v_min + 1));
    }
}