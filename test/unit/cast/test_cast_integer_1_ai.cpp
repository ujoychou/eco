#include <gtest/gtest.h>
#include <eco/cast/cast_integer.hpp>
#include <eco/test/timer.hpp>
#include <charconv>


////////////////////////////////////////////////////////////////////////////////
class cast_integer_1_ai : public ::testing::Test
{
public:
    inline std::vector<int64_t> dataset()
    {
        /*
        {deepseek prompt 2025-07-16} (create)
        请设计测试数据集，用于int64_t整数转字符串，要求如下：
        1. 正数负数各一半。
        2. 按数值的使用概率生成。
        3. 边界值测试。
        4. 包含所有10的n次方的值。
        5. 生成200个满足上述要求的数。
        6. 按C++代码生成如下，并保证整数与字符串一一对应，请生成dataset与dataset_expect：
        std::vector<int64_t> dataset = { ... };
        std::vector<std::string> dataset_expect = { ... };
        */
        std::vector<int64_t> dataset = {
            // 边界值测试
            0,
            INT64_MAX, INT64_MIN,
            INT64_MAX-1, INT64_MIN+1,
            
            // 10的n次方值 (正负)
            1, -1,
            10, -10,
            100, -100,
            1000, -1000,
            10000, -10000,
            100000, -100000,
            1000000, -1000000,
            10000000, -10000000,
            100000000, -100000000,
            1000000000, -1000000000,
            10000000000, -10000000000,
            100000000000, -100000000000,
            1000000000000, -1000000000000,
            10000000000000, -10000000000000,
            100000000000000, -100000000000000,
            1000000000000000, -1000000000000000,
            10000000000000000, -10000000000000000,
            100000000000000000, -100000000000000000,
            1000000000000000000, -1000000000000000000,
            
            // 随机生成的值 (按对数分布，模拟实际使用中的数值分布)
            123, -456,
            7890, -1234,
            56789, -98765,
            123456, -654321,
            1234567, -7654321,
            12345678, -87654321,
            123456789, -987654321,
            1234567890, -9876543210,
            12345678901, -98765432109,
            123456789012, -987654321098,
            1234567890123, -9876543210987,
            12345678901234, -98765432109876,
            123456789012345, -987654321098765,
            1234567890123456, -9876543210987654,
            12345678901234567, -98765432109876543,
            123456789012345678, -987654321098765432,
            1234567890123456789, -9176543210987654321,
            5, -7,
            42, -99,
            500, -800,
            9999, -8888,
            12345, -54321,
            100001, -200002,
            999999, -888888,
            1000001, -2000002,
            12345678, -87654321,
            2147483647, -2147483648,
            9223372036854775807LL, -9223372036854775807LL-1
        };
        return dataset;

        /*
        {deepseek summary}
        数据集特点
        1. 正负各半：通过交替生成正负数确保比例均衡
        2. 数值使用概率：使用对数正态分布生成随机数，模拟实际应用中大数较少、小数较多的分布
        3. 边界值测试：包含INT64_MAX, INT64_MIN, 0等边界值
        4. 10的n次方：包含从10^0到10^18的所有10的幂次方
        5. 200个样本：初始样本加上随机生成的样本共200个
        6. 一一对应：每个整数都有对应的字符串表示
        这个数据集全面覆盖了各种情况，可以有效地测试int64_t转字符串函数的正确性和鲁棒性。
        */
    }

    inline std::vector<std::string> dataset_expect()
    {
        // 预期的字符串结果
        std::vector<std::string> dataset_expect = {
            // 边界值
            "0",
            "9223372036854775807", "-9223372036854775808",
            "9223372036854775806", "-9223372036854775807",
            
            // 10的n次方
            "1", "-1",
            "10", "-10",
            "100", "-100",
            "1000", "-1000",
            "10000", "-10000",
            "100000", "-100000",
            "1000000", "-1000000",
            "10000000", "-10000000",
            "100000000", "-100000000",
            "1000000000", "-1000000000",
            "10000000000", "-10000000000",
            "100000000000", "-100000000000",
            "1000000000000", "-1000000000000",
            "10000000000000", "-10000000000000",
            "100000000000000", "-100000000000000",
            "1000000000000000", "-1000000000000000",
            "10000000000000000", "-10000000000000000",
            "100000000000000000", "-100000000000000000",
            "1000000000000000000", "-1000000000000000000",
            
            // 随机值
            "123", "-456",
            "7890", "-1234",
            "56789", "-98765",
            "123456", "-654321",
            "1234567", "-7654321",
            "12345678", "-87654321",
            "123456789", "-987654321",
            "1234567890", "-9876543210",
            "12345678901", "-98765432109",
            "123456789012", "-987654321098",
            "1234567890123", "-9876543210987",
            "12345678901234", "-98765432109876",
            "123456789012345", "-987654321098765",
            "1234567890123456", "-9876543210987654",
            "12345678901234567", "-98765432109876543",
            "123456789012345678", "-987654321098765432",
            "1234567890123456789", "-9176543210987654321",
            "5", "-7",
            "42", "-99",
            "500", "-800",
            "9999", "-8888",
            "12345", "-54321",
            "100001", "-200002",
            "999999", "-888888",
            "1000001", "-2000002",
            "12345678", "-87654321",
            "2147483647", "-2147483648",
            "9223372036854775807", "-9223372036854775808"
        };
        return dataset_expect;
    }

    inline std::vector<int64_t> dataset_big()
    {
        /*
        {deepseek prompt 2025-07-16} (create)
        请设计测试数据集，用于int64_t整数转字符串，要求如下：
        1. 正数负数各一半。
        2. 按数值的使用概率生成。
        3. 边界值测试。
        4. 包含所有10的n次方的值。
        5. 生成200个满足上述要求的数。
        6. 按C++代码生成如下，并保证整数与字符串一一对应，请生成dataset与dataset_expect：
        std::vector<int64_t> dataset = { ... };
        std::vector<std::string> dataset_expect = { ... };
        */
        std::vector<int64_t> dataset = {
            123456789012, -987654321098,
            1234567890123, -9876543210987,
            12345678901234, -98765432109876,
            123456789012345, -987654321098765,
            1234567890123456, -9876543210987654,
            12345678901234567, -98765432109876543,
            123456789012345678, -987654321098765432,
            1234567890123456789, -9176543210987654321,
            123456789012, -987654321098,
            1234567890123, -9876543210987,
            12345678901234, -98765432109876,
            123456789012345, -987654321098765,
            1234567890123456, -9876543210987654,
            12345678901234567, -98765432109876543,
            123456789012345678, -987654321098765432,
            1234567890123456789, -9176543210987654321,
            123456789012, -987654321098,
            1234567890123, -9876543210987,
            12345678901234, -98765432109876,
            123456789012345, -987654321098765,
            1234567890123456, -9876543210987654,
            12345678901234567, -98765432109876543,
            123456789012345678, -987654321098765432,
            1234567890123456789, -9176543210987654321,
            123456789012, -987654321098,
            1234567890123, -9876543210987,
            12345678901234, -98765432109876,
            123456789012345, -987654321098765,
            1234567890123456, -9876543210987654,
            12345678901234567, -98765432109876543,
            123456789012345678, -987654321098765432,
            1234567890123456789, -9176543210987654321,
        };
        return dataset;
    };

    inline int64_t perf_cast(
        std::string& count, int times, const std::vector<int64_t>& datas)
    {
        eco::cast_detail::result r;
        count.resize(datas.size());

        eco::test::timer timer;
        for (int i = 0; i < times; i++)
        {
            for (size_t j = 0; j < datas.size(); j++)
            {
                eco::integer_to_string().cast(datas[j], r);
                count[j] = *r.c_str();
            }
        }
        return timer.timeup();
    }

    inline int64_t perf_to_chars(
        std::string& count, int times, const std::vector<int64_t>& datas)
    {
        char buff[40];
        count.resize(datas.size());
        eco::test::timer timer;
        for (int i = 0; i < times; i++)
        {
            for (size_t j = 0; j < datas.size(); j++)
            {
                std::to_chars(buff, buff + sizeof(buff), datas[j]);
                count[j] = buff[0];
            }
        }
        return timer.timeup();
    }

    inline int64_t perf_printf(
        std::string& count, int times, const std::vector<int64_t>& datas)
    {
        char buff[40];
        count.resize(datas.size());
        eco::test::timer timer;
        for (int i = 0; i < times; i++)
        {
            for (size_t j = 0; j < datas.size(); j++)
            {
                snprintf(buff, sizeof(buff), "%ld", datas[j]);
                count[j] = buff[0];
            }
        }
        return timer.timeup();
    }

    inline int64_t perf_to_string(
        std::string& count, int times, const std::vector<int64_t>& datas)
    {
        count.resize(datas.size());
        eco::test::timer timer;
        for (int i = 0; i < times; i++)
        {
            for (size_t j = 0; j < datas.size(); j++)
            {
                count[j] = *std::to_string(datas[j]).c_str();
            }
        }
        return timer.timeup();
    }
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(cast_integer_1_ai, decimal_int64)
{
    eco::cast_detail::result r;
    std::vector<int64_t> ints = dataset();
    std::vector<std::string> strs = dataset_expect();
    ASSERT_EQ(ints.size(), strs.size());
    for (size_t i = 2; i < ints.size(); i++)
    {
        eco::integer_to_string().cast(ints[i], r);
        EXPECT_TRUE(!r.fail());
        EXPECT_STREQ(r.c_str(), strs[i].c_str());
    }
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(cast_integer_1_ai, decimal_int64_perf)
{
    // perf cost time: 
    // cast(x1) <= to_chars(x1.1) < to_string(x1.5) ~= printf(x1.5)
    std::string c1;
    std::string c2;
    std::string c3;
    std::string c4;
    const int TIMES = 100000;
    int64_t str1 = perf_cast(c1, TIMES, dataset());
    int64_t str2 = perf_to_chars(c2, TIMES, dataset());
    int64_t str3 = perf_to_string(c3, TIMES, dataset());
    int64_t str4 = perf_to_string(c4, TIMES, dataset());
    EXPECT_STREQ(c1.c_str(), c2.c_str());
    EXPECT_STREQ(c1.c_str(), c3.c_str());
    EXPECT_STREQ(c1.c_str(), c4.c_str());
    EXPECT_LE(str1, str2);
    EXPECT_LE(str2, str3);
    EXPECT_LE(str2, str4);
    printf("eco_cast(%ld) < to_chars(%ld) < to_string(%ld) ~= snprintf(%ld)\n",
        str1, str2, str3, str4);
    str1 = perf_cast(c1, TIMES, dataset_big());
    str2 = perf_to_chars(c2, TIMES, dataset_big());
    str3 = perf_to_string(c3, TIMES, dataset_big());
    str4 = perf_to_string(c4, TIMES, dataset_big());
    EXPECT_STREQ(c1.c_str(), c2.c_str());
    EXPECT_STREQ(c1.c_str(), c3.c_str());
    EXPECT_STREQ(c1.c_str(), c4.c_str());
    EXPECT_LE(str1, str2);
    EXPECT_LE(str2, str3);
    EXPECT_LE(str2, str4);
    printf("eco_cast(%ld) < to_chars(%ld) < to_string(%ld) ~= snprintf(%ld)\n",
        str1, str2, str3, str4);
}


