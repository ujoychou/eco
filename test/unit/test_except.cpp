#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/except.hpp>


////////////////////////////////////////////////////////////////////////////////
bool    ARGS_0 = true;
int     ARGS_1 = 231;
float   ARGS_2 = 232.125;
double  ARGS_3 = 233.5674;
const char* ARGS_4 = "string-x";
const char* ARGS = "1;231;232.13;233.567";
const char* DIVISOR_PRINTF = "Divisor cannot be zero: %d/%d";
const char* DIVISOR_WHAT = "Divisor cannot be zero: 10/0";
const int   USER_EC = 8021;
const char* USER_NAME = "zhangsan";
const char* USER_PRINTF = "[%s] not exist = ";
const char* USER_WHAT = "[zhangsan] not exist = 8021";
const char* USER_PASS = "abc123abc456";
const int   SQL_EC = 8023;
const char* SQL_QUERY = "select * from user where id = 1";
const char* SQL_WHAT = "[sql error #8023]: select * from user where id = 1";
const char* SQL_PRINTF = "[sql error #%d]: ";
const char* SQL_FORMAT = "[sql error #{}]: ";
////////////////////////////////////////////////////////////////////////////////
class test_except : public ::testing::Test
{
public:
    enum
    {
        E1001 = 1001,
        E1002 = 1002,
        E1003 = 1003,
        E1004 = 1004,

        // user is empty or invalid: %s.
        E2001 = 2001,
        // password is empty.
        E2002 = 2002,
        // sql query from database error: [%s] (%d)
        E2003 = 2003,
        // login error because of query password.
        E2004 = 2004,
        // user and password unmatch.
        E2010 = 2010,
    };

public:
    inline int div(int a, int b)
    {
        eco_except(error, E1001, !b, INT32_MIN).printf(DIVISOR_PRINTF, a, b);
        return a / b;
    }

    inline int div_throw(int a, int b)
    {
        eco_throw(error, E1001, !b).printf(DIVISOR_PRINTF, a, b);
        return a / b;
    }

public:
    inline std::string query_password(const std::string& user, int m)
    {
        eco_except(error, E2003, m == 3, std::string())
            .printf(SQL_PRINTF, SQL_EC) << SQL_QUERY;
        return m == 4 ? "" : USER_PASS;
    }

    inline eco::bool_t login(
        const std::string& user,
        const std::string& pass, int m)
    {
        eco_except(info, E2001, m == 1).args(user)
            .printf(USER_PRINTF, user.c_str());
        eco_except(info, E2002, m == 2);
        std::string pass_query = query_password(user, m);
        eco_except(error, E2004, pass_query.empty());
        eco_except(info, E2010, pass_query != pass && m == 10);
        return true;
    }
};



////////////////////////////////////////////////////////////////////////////////
TEST_F(test_except, format)
{
    // eco::printf
    eco_except(info, E1001).printf(USER_PRINTF, USER_NAME) << USER_EC;
    EXPECT_EQ(eco::except::this_except().id(), E1001);
    EXPECT_STREQ(eco::except::this_except().what(), USER_WHAT);
    // eco::format
    eco_except(error, E1002).format(SQL_FORMAT, SQL_EC) << SQL_QUERY;
    EXPECT_EQ(eco::except::this_except().id(), E1002);
    EXPECT_TRUE(eco::except::this_except().what() == SQL_WHAT);
    // eco::args
    eco_except(info, E1003).args(ARGS_0).args(ARGS_1).args(ARGS_2)
        .args(ARGS_3);
    EXPECT_EQ(eco::except::this_except().id(), E1003);
    EXPECT_TRUE(eco::except::this_except().args() == ARGS);
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(test_except, logs)
{
    // eco_error: printf style
    eco_except(debug, E1001).printf(USER_PRINTF, USER_NAME) << USER_EC;
    EXPECT_EQ(eco::except::this_except().id(), E1001);
    EXPECT_TRUE(eco::except::this_except().what() == std::string());
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(test_except, div)
{
    int c = div(10, 2);
    EXPECT_TRUE(c == 5);
    c = div(10, 5);
    EXPECT_TRUE(c == 2);
    c = div(10, 0);
    EXPECT_TRUE(c == INT32_MIN);
}
TEST_F(test_except, div_throw)
{
    try
    {
        int c = div(10, 2);
        EXPECT_TRUE(c == 5);
        c = div(10, 5);
        EXPECT_TRUE(c == 2);
        c = div(10, 0);
        EXPECT_FALSE(c | false);
    }
    catch (const eco::except& e)
    {
        EXPECT_EQ(e.id(), E1001);
        EXPECT_STREQ(e.what(), DIVISOR_WHAT);
    }
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(test_except, login)
{
    // 场景
    // 登录失败可能存在多种场景，如下
    // 1. 用户输入信息为空。（业务错误，用户需重新输入）
    // 2. 用户与密码不匹配。（业务错误，用户需重新输入）
    // 3. 用户未注册，需要管理员添加用户。（业务错误，管理员添加）
    // 4. 数据库访问失败，无法校验。（系统错误，需要软件提供商修复）

    // 方案
    // 登录失败的场景统一使用eco::except来处理。
    // 1. 业务错误（用户可恢复），由except返回具体错误码，以及详细消息与参数列表。
    // 2. 系统错误（用户不可恢复），由except返回具体错误码，以及详细消息与参数列表。
    // 3. 错误类型划分：如业务错误、系统错误。
    // 3.1. 业务错误：日志应该是info，但不是error；前端展示为非法输入；
    // 3.2. 业务错误：日志可以是warn，如：此异常为业务预警；
    // 3.3. 系统错误：日志应该是error；前端展示为系统错误；
    // 3.4. 系统错误：日志可以是fatal，如：此错误可能导致业务数据混乱，需要退出该程序；
    // 4. 错误类型举例：
    // * 业务错误：资金余额不足，无法委托下单。(eco：：info)
    // * 业务错误：风控R23校验失败，无法委托下单。（eco::warn | 风控预警功能）
    // * 系统错误：期货合约不存在，无法委托下单。（eco::error）
    
    eco::bool_t result = login(USER_NAME, USER_PASS, 1);
    eco::except except = eco::except::this_except();
    EXPECT_FALSE(result);
    EXPECT_EQ(except.id(), E2001);
    EXPECT_EQ(except.what(), USER_WHAT);
    result = login(USER_NAME, USER_PASS, 2);
    EXPECT_FALSE(result);
    EXPECT_EQ(except.id(), E2002);
    EXPECT_EQ(except.what(), "");
    result = login(USER_NAME, USER_PASS, 3);
    EXPECT_FALSE(result);
    EXPECT_EQ(except.id(), E2003);
    EXPECT_EQ(except.what(), SQL_WHAT);
    result = login(USER_NAME, USER_PASS, 4);
    EXPECT_FALSE(result);
    EXPECT_EQ(except.id(), E2004);
    EXPECT_EQ(except.what(), "");
    result = login(USER_NAME, USER_PASS, 10);
    EXPECT_FALSE(result);
    EXPECT_EQ(except.id(), E2010);
    EXPECT_EQ(except.what(), "");
}