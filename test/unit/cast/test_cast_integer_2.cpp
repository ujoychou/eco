#include <gtest/gtest.h>
#include <eco/cast/cast_integer.hpp>


////////////////////////////////////////////////////////////////////////////////
class cast_integer_2 : public ::testing::Test
{
public:
    inline void error_1()
    {
        eco::string_to_integer<int> a("");
        ASSERT_EQ(a.value, 0);
        ASSERT_EQ(a.error, -1);
        ASSERT_EQ(a.end, nullptr);
        ASSERT_FALSE(a.cast("+"));
        ASSERT_EQ(a.value, 0);
        ASSERT_EQ(a.error, -1);
        ASSERT_EQ(a.end, nullptr);
        ASSERT_FALSE(a.cast("-"));
        ASSERT_EQ(a.value, 0);
        ASSERT_EQ(a.error, -1);
        ASSERT_EQ(a.end, nullptr);
    }

    inline void error_2()
    {
        eco::string_to_integer<uint32_t> a("-1");
        ASSERT_EQ(a.value, 0u);
        ASSERT_EQ(a.error, -2);
        ASSERT_EQ(a.end, nullptr);
        eco::string_to_integer<uint64_t> b("-1");
        ASSERT_EQ(b.value, 0u);
        ASSERT_EQ(b.error, -2);
        ASSERT_EQ(a.end, nullptr);
    }

    inline void error_3()
    {
        eco::string_to_integer<int> a("a", 1);
        ASSERT_EQ(a.value, 0);
        ASSERT_EQ(a.error, -3);
        ASSERT_EQ(a.end, nullptr);
        ASSERT_FALSE(a.cast("123", 37));
        ASSERT_EQ(a.value, 0);
        ASSERT_EQ(a.error, -3);
        ASSERT_EQ(a.end, nullptr);
    }

    template<typename int_t>
    inline void error_4(const char* c, int base, int_t v, const char* end)
    {
        eco::string_to_integer<int_t> a(c, base);
        ASSERT_EQ(a.error, -4);
        ASSERT_EQ(a.value, v);
        ASSERT_EQ(a.end, end);
    }

    inline eco::bool_t signed_string(const char* c)
    {
        for (; *c && eco::empty(*c); ++c) {}
        return (*c == '-');
    }

    template<typename int_t>
    inline void error_5(const char* c, int base)
    {
        eco::string_to_integer<int_t> a(c, base);
        ASSERT_EQ(a.error, -5);
        if (signed_string(c))
            ASSERT_EQ(a.value, std::numeric_limits<int_t>::min());
        else
            ASSERT_EQ(a.value, std::numeric_limits<int_t>::max());
        ASSERT_EQ(a.end, nullptr);
    }


public:
    inline std::vector<int> split(const char* input, int base)
    {
        std::vector<int> result;
        eco::string_to_integer<int> a(input, base);
        if (a.end != input)
        {
            result.push_back(a.value);
        }
        while (a.end != nullptr)
        {
            a.end = a.next(a.end, base);
            if (a.end != NULL)
            {
                a.cast(a.end, base);
                result.push_back(a.value);
            }
        }
        return result;
    }

    inline void zero(int base)
    {
        eco::string_to_integer<int> a("+0", base);
        ASSERT_EQ(a.value, 0);
        ASSERT_EQ(a.error, 0);
        ASSERT_EQ(a.end, nullptr);
        ASSERT_TRUE(a.cast("-0", base));
        ASSERT_EQ(a.value, 0);
        ASSERT_EQ(a.error, 0);
        ASSERT_EQ(a.end, nullptr);
        ASSERT_TRUE(a.cast("0", base));
        ASSERT_EQ(a.value, 0);
        ASSERT_EQ(a.error, 0);
        ASSERT_EQ(a.end, nullptr);
    }

    template<typename int_t>
    inline void value(int base, const char* c, int_t v)
    {
        eco::string_to_integer<int_t> a(c, base);
        ASSERT_EQ(a.value, v);
        ASSERT_EQ(a.error, 0);
        ASSERT_EQ(a.end, nullptr);
    }
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(cast_integer_2, error)
{
    this->error_1();
    this->error_2();
    this->error_3();

    // base = 10
    const char* c = "abc";
    this->error_4<int>(c, 10, 0, c);
    c = "  00123456789  ";
    this->error_4<int>(c, 10, 123456789, c + 13);
    c = "  -00123456789  ";
    this->error_4<int>(c, 10, -123456789, c + 14);
    // base = 2
    c = "  0012345";
    this->error_4<int>(c, 2, 1, c + 5);
    c = "  -0012345";
    this->error_4<int>(c, 2, -1, c + 6);
    // base = 8
    c = "  0012345678";
    this->error_4<int>(c, 8, 01234567, c + 11);
    c = "  -0012345678";
    this->error_4<int>(c, 8, -01234567, c + 12);
    // base = 16
    c = "  00123456789abcdefg";
    this->error_4<int64_t>(c, 16, 0x123456789abcdefull, c + 19);
    c = "  -00123456789abcdefg";
    this->error_4<int64_t>(c, 16, -0x123456789abcdefull, c + 20);

    // base = 10
    this->error_5<int32_t> ("2147483648", 10);
    this->error_5<uint32_t>("4294967296", 10);
    this->error_5<int64_t> ("9223372036854775808", 10);
    this->error_5<uint64_t>("18446744073709551616", 10);
    // base = 16
    this->error_5<int32_t> ("80000000", 16); 
    this->error_5<uint32_t>("100000000", 16); 
    this->error_5<int64_t> ("8000000000000000", 16); 
    this->error_5<uint64_t>("10000000000000000", 16);
    // base = 8
    this->error_5<int32_t> ("20000000000", 8);
    this->error_5<uint32_t>("40000000000", 8);
    this->error_5<int64_t> ("1000000000000000000000", 8);
    this->error_5<uint64_t>("2000000000000000000000", 8);
    // base = 2
    this->error_5<int32_t> ("10000000000000000000000000000000", 2);
    this->error_5<uint32_t>("100000000000000000000000000000000", 2);
    this->error_5<int64_t> ("1000000000000000000000000000000000000000000000000000000000000000", 2);
    this->error_5<uint64_t>("10000000000000000000000000000000000000000000000000000000000000000", 2);
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(cast_integer_2, split)
{
    // base = 10
    std::vector<int> result = this->split("123 456 789", 10);
    ASSERT_EQ(result.size(), 3u);
    ASSERT_EQ(result[0], 123);
    ASSERT_EQ(result[1], 456);
    ASSERT_EQ(result[2], 789);
    result = this->split(";123;456;789;", 10);
    ASSERT_EQ(result.size(), 3u);
    ASSERT_EQ(result[0], 123);
    ASSERT_EQ(result[1], 456);
    ASSERT_EQ(result[2], 789);
    result = this->split("123abc;456bcd;789efg;", 10);
    ASSERT_EQ(result.size(), 3u);
    ASSERT_EQ(result[0], 123);
    ASSERT_EQ(result[1], 456);
    ASSERT_EQ(result[2], 789);

    // base = 2
    result = this->split("101 110 111 ", 2);
    ASSERT_EQ(result.size(), 3u);
    ASSERT_EQ(result[0], 0b101);
    ASSERT_EQ(result[1], 0b110);
    ASSERT_EQ(result[2], 0b111);
    result = this->split(" 1014 1103 1112 ", 2);
    ASSERT_EQ(result.size(), 3u);
    ASSERT_EQ(result[0], 0b101);
    ASSERT_EQ(result[1], 0b110);
    ASSERT_EQ(result[2], 0b111);
    // base = 8
    result = this->split("123 456 789", 8);
    ASSERT_EQ(result.size(), 3u);
    ASSERT_EQ(result[0], 0123);
    ASSERT_EQ(result[1], 0456);
    ASSERT_EQ(result[2], 07);
    result = this->split(" 12389 456abc 78edf ", 8);
    ASSERT_EQ(result.size(), 3u);
    ASSERT_EQ(result[0], 0123);
    ASSERT_EQ(result[1], 0456);
    ASSERT_EQ(result[2], 07);
    // base = 16
    result = this->split("12345 6789a bcedf", 16);
    ASSERT_EQ(result.size(), 3u);
    ASSERT_EQ(result[0], 0x12345);
    ASSERT_EQ(result[1], 0x6789a);
    ASSERT_EQ(result[2], 0xbcedf);
    result = this->split("@12345;; 6789a&^ bcedf!@", 16);
    ASSERT_EQ(result.size(), 3u);
    ASSERT_EQ(result[0], 0x12345);
    ASSERT_EQ(result[1], 0x6789a);
    ASSERT_EQ(result[2], 0xbcedf);
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(cast_integer_2, normal)
{
    // zero
    this->zero(0);
    this->zero(10);
    this->zero(2);
    this->zero(8);
    this->zero(16);

    // max/min value
    this->value<int32_t>(10, "2147483647", INT32_MAX);
    this->value<int32_t>(10, "-2147483648", INT32_MIN);
    this->value<uint32_t>(10, "4294967295", UINT32_MAX);
    this->value<int64_t>(10, "9223372036854775807", INT64_MAX);
    this->value<int64_t>(10, "-9223372036854775808", INT64_MIN);
    this->value<uint64_t>(10, "18446744073709551615", UINT64_MAX);

    // positive
    this->value(0, "123", 123);
    this->value(0,  "    123", 123);
    this->value(0,  "    0000123", 0123);
    this->value(10, "123", 123);
    this->value(10, "    0000123", 123);
    this->value(8,  "123", 0123);
    this->value(8,  "    0000123", 0123);
    this->value(16, "123", 0x123);
    this->value(16, "    0000123", 0x123);
    this->value(2,  "1101", 0b1101);
    this->value(2,  "    00001101", 0b1101);

    // negative
    this->value(0, "-123", -123);
    this->value(0, "    -123", -123);
    this->value(0, "    -0000123", -0123);
    this->value(10, "-123", -123);
    this->value(10, "    -0000123", -123);
    this->value(8, "-123", -0123);
    this->value(8, "    -0000123", -0123);
    this->value(16, "-123", -0x123);
    this->value(16, "    -0000123", -0x123);
    this->value(2, "-1101", -0b1101);
    this->value(2, "    -00001101", -0b1101);
}
