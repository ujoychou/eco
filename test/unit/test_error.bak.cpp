#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/error.hpp>
#include <limits>


////////////////////////////////////////////////////////////////////////////////
class TestError : public ::testing::Test
{
public:
    enum
    {
        e_user_empty                = 1001,
        e_password_empty            = 1002,
        e_user_unexist              = 2001,
        e_password_unmatch          = 2002,
        e_user_pwd_invalid          = 2003,
        e_user_pwd_error            = 2004,
        e_query_user                = 3004,
        e_query_password            = 3005,
    };

    struct return_control
    {
        bool f11_user_exist;
        bool f11_password_unmatch;
        bool f2_query_password;
    };

    #define eco_ui(x, y)

    inline void ui_login(const std::string& user, const std::string& password)
    {
        if (!f1_login(user, password))
        {
            if (eco::error().sys())
            {
                eco_ui(error, "用户登录失败：系统故障，系统提供商正在解决，大概需要5小时。");
                return;
            }
            if (eco::error().id() == e_user_pwd_invalid)
            {
                eco_ui(error, "用户登录失败：用户未注册或者密码不正确");
                return;
            }
        }
    }

    inline bool f1_login(const std::string& user, const std::string& password)
    {
        // check user and password empty
        if (user.empty())
        {
            eco_error(e_user_empty, "user name is empty");
            return false;
        }
        if (password.empty())
        {
            eco_error(e_password_empty, "password is empty");
            return false;
        }
        
        // business: whether user is exist
        eco::result rc = f11_user_exist(user);
        // error when check business
        if (rc == eco::result_error)
        {
            eco_error(e_user_pwd_error).sys(0) << "login check error";
            return false;
        }
        if (rc == eco::result_false)
        {
            eco_error(e_user_pwd_invalid, "user is not exist");
            return false;
        }
        
        // business: whether password is match
        eco::result rc = f12_check_password(user, password);
        if (rc == eco::result::error)
        {
            return false;
        }
        if (rc == eco::result::fail)
        {
            eco_error(e_user_pwd_invalid) << "password unmatch";
            return false;
        }
        return true;
    }

    inline eco::result f11_user_exist(const std::string& user)
    {
        eco::error::clear();

        // database error
        if (!f2_query_user(user))
        {
            return eco::result::error;
        }
        // business error
        if (!ctrl.f11_user_exist)
        {
            return eco::result::fail;
        }
        return eco::result::ok;
    }


    inline eco::result f12_check_password(const std::string& user, const std::string& pwd)
    {
        // database error
        std::string current_pwd;
        if (!f2_query_password(user, current_pwd))
        {
            return eco::result::error;
        }
        // business error
        if (ctrl.f11_password_unmatch)
        {
            return eco::result::fail;
        }
        return eco::result::ok;
    }

    inline bool f2_query_user(const std::string&)
    {
        if (ctrl.f2_query_user)
        {
            const char* sql = "select * from account where user=";
            eco_error(e_query_user, "database sql error: %s", sql);
            return false;
        }
        return true;
    }

    inline bool f3_query_password(std::string& pwd, const std::string& user)
    {
        if (ctrl.f2_query_password)
        {
            const char* sql = "select * from account where user= or pwd=";
            eco_error(e_query_password).sys(true) << "database sql error: " << sql;
            return false;
        }
        return true;
    }

public:
    return_control ctrl;
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(TestError, format)
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
        args(v1).args(v2, 4, true).args(v2, 2).args(v4);
    ASSERT_EQ(eco_error().message(),
        "argc1=3.14 argc2=pi-value-is argc3=20241030 argc4=3.1416");
}
class error_data
{
	int id;
	eco::string path;
	eco::string message;

private:
    const char* format;
	eco::string argvs;

private:
    eco::string stack;

    inline error_data& format(const char* v)
	{
        format = 0;
	}

    template<typename type_t>
	inline error_data& args(const type_t& v)
	{
        argvs << v;		
	}
    template<typename type_t>
    inline error_data& operator % (const type_t& v)
    {
        argvs << v;
        argvs.append('\0');
    }

    inline const std::string& message()
    {
        if (format != NULL)
        {
            message = format;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
TEST_F(TestError, return_error)
{
    if (!user_login())
    {
        eco_error();
        return false;
    }
    
    //const char* url = "10.21.153.55:9050/images/icon/cool.jpg";
    //eco::error("login fail, username or password is error.");
    //eco::error("get file error: %1 don't have %2 privilege of file %3") % "zhouyu" % 1 % url;
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(TestError, throw_error)
{
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(TestError, multiple_language)
{
    /* <en_us/error.json>
    {
        "trade": {
            "subaccount": {
                "401": "The sub-account %1 does not exist, which hasn't been registered."
                "402": "The balance of sub-account %1 is insufficient. The current balance is %2, which is less than the %3 required for this transaction."
            }
        }
    }
    */


    try
    {
        eco_throw("/trade/subaccount/401") % "S90092" % 1000 % 1300;
    }
    catch(const eco::error& e)
    {
        ASSERT_TRUE(e.message() == "The balance of sub-account S90092 is "
"insufficient. The current balance is 1000, which is less than the 1300 "
"required for this transaction.");
    }
}


void app()
{
    const char* product = "apple";
    if (!service_trade->buy(product))
    {
        // [用户自行解决]
        // 编码模式
        if (error().id() == 1001)
        {
            eco_throw(1001, "购买商品[%s]失败。当前购买人数太多，请稍后再试。", product);
            eco_throw().prompt("购买商品[%s]失败", product);
            eco_throw().action(ACTION_WAITTIME, "当前购买人数太多，请稍后再试。");
        }
        else if (error().id() == 1002)
        {
            eco_throw().prompt("购买商品[%s]失败", product);
            eco_throw().action(ACTION_RECHARGE, "资金余额不足，请充值。");
        }
        else if (error().path() == "trade/1003")
        {
            eco_throw().prompt("购买商品[%s]失败", product);
            eco_throw().action(ACTION_CONTACT_PROVIDER, "当前购买人数太多，后台系统资源不足，软件提供商正在解决中，预计50分钟后解决。");
        }

        // 配置模式
        eco_throw() % product;
        eco_throw() % product;

        // [软件商解决]
        // 编码模式
        eco_throw(1001, "购买商品[%s]失败。软件提供商正在解决中，预计50分钟后解决。", product)
            .reason("当前购买人数太多，后台系统资源不足");
        
        // 配置模式
        eco_throw(1001) % user_name;
    }

    // 显示给用户：错误提示与错误原因
    ui.set_prompt(error().prompt());
    ui.set_reason(error().reason());
    ui.set_action(ACTION_CONTACT_PROVIDER);
    ui.set_action(error().id(), ACTION_RECHARGE);
    ui.show();
}


















void test()
{
    /* [异常管理]
    1. 支持异常ID与PATH
    2. 支持
    */
}