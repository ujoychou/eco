#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/error.hpp>
#include <limits>


////////////////////////////////////////////////////////////////////////////////
class TestThrow : public ::testing::Test
{
public:
    struct return_control
    {
        bool f11_user_exist;
        bool f11_password_unmatch;
        bool f2_query_password;
    };

    inline void ui_login(const std::string& user, const std::string& password)
    {
        try
        {
            f1_login();
        }
        catch(const eco::error& e)
        {
            if (e.sys())
            {
                eco_ui(error, "用户登录失败：系统故障，系统提供商正在解决，大概需要5小时。");
            }
            else if (e.id() == e_user_unexist || e.id() == e_user_pwd_invalid)
            {
                eco_ui(error, "用户登录失败：用户未注册或者密码不正确");
            }
            else if (e.id() == e_user_empty)
            {
                eco_ui(error, "用户登录失败：用户名不能为空");
            }
            else if (e.id() == e_password_empty)
            {
                eco_ui(error, "用户登录失败：用户密码不能为空");
            }
        }
    }

public:
    inline void f1_login(const std::string& user, const std::string& password)
    {
        // check user and password empty
        if (user.empty())
        {
            eco_throw(e_user_empty, "user name is empty");
        }
        if (password.empty())
        {
            eco_throw(e_password_empty, "password is empty");
        }
        
        // business: whether user is exist
        if (!f11_user_exist(user))
        {
            eco_throw(e_user_pwd_error);
        }
        
        // business: whether password is match
        if (!f12_check_password(user, password))
        {
            eco_throw(e_user_pwd_error);
        }
    }

    inline bool f11_user_exist(const std::string& user)
    {
        f2_query_user(user);
        return (ctrl.f11_user_exist);
    }


    inline bool f12_check_password(const std::string& user, const std::string& pwd)
    {
        std::string current_pwd;
        f2_query_password(user, current_pwd);
        return (!ctrl.f11_password_unmatch);
    }

    inline void f2_query_user(const std::string&)
    {
        if (ctrl.f2_query_user)
        {
            const char* sql = "select * from account where user=";
            eco_throw(e_query_user, "database sql error: %s", sql).sys(true);
        }
    }

    inline void f3_query_password(std::string& pwd, const std::string& user)
    {
        if (ctrl.f2_query_password)
        {
            const char* sql = "select * from account where user= or pwd=";
            eco_throw(e_query_password).sys(true) << "database sql error: " << sql;
        }
    }

public:
    return_control ctrl;
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(TestThrow, format)
{
    int v1 = 20241030;
    float v2 = 3.1415926
    double v3 = 3.1415926
    const char* v4 = "pi-value-is";
    // arg setting: using %
    eco_error(1001, "argc1=%1 argc2=%2 argc3=%3 argc4=%4")
        % v1 % eco::precision(v2, 4, true) %  eco::precision(v2, 2) % v4;
    ASSERT_EQ(eco_error().message(),
        "argc1=20241030 argc2=3.1416 argc3=3.14 argc4=pi-value-is");
    // arg setting
    eco_error(1001, "argc1=%3 argc2=%1 argc3=%4 argc4=%2").
        arg(v1).arg(v2, 4, true).arg(v2, 2).arg(v4);
    ASSERT_EQ(eco_error().message(),
        "argc1=3.14 argc2=pi-value-is argc3=20241030 argc4=3.1416");
}