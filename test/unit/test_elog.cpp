#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/log.hpp>


////////////////////////////////////////////////////////////////////////////////
class test_elog : public ::testing::Test
{
public:
    inline void log_level()
    {
        eco_log(debug);
        eco_log(info);
        eco_log(warn);
        eco_log(error);
    }

    inline void log_format()
    {
        int a = 0, b = 1, c = 2;
        // output: format, printf, <<
        eco_log(debug).format("a=% < b=%") % a % b << c;
        eco_log(debug).printf("a=%d < b=%d", a, b) << c;
        eco_log(debug) << "a=" << a << " < b=" << b << c;

        // title: function, module, flag, etc.
        eco_log(debug).format("a=% < b=%") % a % b << c;
        eco_log(debug).printf("a=%d < b=%d", a, b) << c;
        eco_log(debug) << "a=" << a << " < b=" << b << c;
    }

    inline void log_cond()
    {
        int a = 0, b = 1;

        // when: condition
        /*
        eco_log(debug, a < b).format("a=% < b=%") % a % b;
        eco_log(debug, a < b).printf("a=%d < b=%d", a, b);
        eco_log(debug, a < b) << "a=" << a << " < b=" << b;
        eco_log(debug, a < b);
        */

        // each: interval
        eco_log(debug, 3, 1).format("a=% < b=%") % a % b;
        eco_log(debug, 3, 1).printf("a=%d < b=%d", a, b);
        eco_log(debug, 3, 1) << "a=" << a << " < b=" << b;
        eco_log(debug, 3, 1);

        // when_each
        eco_log(debug, a < b, 3, 0).format("a=% < b=%") % a % b;
        eco_log(debug, a < b, 3, 0).printf("a=%d < b=%d", a, b);
        eco_log(debug, a < b, 3, 0) << "a=" << a << " < b=" << b;
        eco_log(debug, a < b, 3, 0);
    }

    inline void log_aspect()
    {
        // (21321) <log_aspect> ::: @zhouyu local and remote entity.
        eco_log(debug) << "local & remote aspect";

        // (21321) <entity> ::: @zhouyu local and remote entity.
        eco_log(debug).aspect("entity") << "local & remote entity";
        eco_log(debug).aspect("security") << "security & authorization";
        eco_log(debug).aspect("discovery") << "spdp & sedp";
    }
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(test_elog, elog)
{
    eco::log::config conf;
    // cache and async
    conf.cache_size(4 * 1024 * 1024).entry_size(1024).async(true);
    // format
    conf.modula("service").format(
        "{time} (thread) [level] (chain) <modula.aspect> "
        "mode @user message {pos} {#error}");
    conf.sink("file").path("./log/")
        .file("elog_{time}_{pc}_{pid}.log")
        .roll_size(100 * 1024 * 1024)
        .level(eco::log::debug);
    conf.sink("console").level(eco::log::info)
        .level(1, eco::log::debug)
        .level(2, eco::log::info)
        .level(3, eco::log::warn);
    conf.sink("monitor").level(eco::log::warn)
        .level(1, eco::log::debug)
        .level(2, eco::log::info)
        .level(3, eco::log::warn);
    eco::log::elog::start(conf);

    // logging test
    log_level();
    log_format();
    log_cond();
    log_aspect();
}


////////////////////////////////////////////////////////////////////////////////
// 金融系统：交易/行情/合约/结算/风控
#define trader_log(level, ...)  eco_log_modula(1, "trader", level, __VA_ARGS__)
#define market_log(level, ...)  eco_log_modula(2, "market", level, __VA_ARGS__)
#define target_log(level, ...)  eco_log_modula(3, "target", level, __VA_ARGS__)
#define settle_log(level, ...)  eco_log_modula(4, "settle", level, __VA_ARGS__)
#define riskctrl_log(level, ...)  eco_log_modula(5, "riskctrl", level, __VA_ARGS__)
////////////////////////////////////////////////////////////////////////////////
TEST_F(test_elog, modula)
{
    int rc = 0;
    // [debug] (...) <trader> --- buy 100 shares of AAPL.
    trader_log(debug).mode(eco::log::mode_req) << "buy 100 shares of AAPL";
    // [debug] (...) <trader.order> ::: @ujoy buy 100 shares of AAPL.
    trader_log(debug).aspect("order").user("ujoy") << "buy 100 shares of AAPL";
    trader_log(debug).aspect("order").user("ujoy").printf(
        "buy %d shares of AAPL", 100);
    trader_log(debug).aspect("order").user("ujoy").format(
        "buy {} shares of AAPL", 100);
    // [error] (...) <trader.order> === @ujoy buy 100 shares of AAPL failed. 
    // {unit/test_elog.cpp:124} {#-1}
    trader_log(error, rc != 0).aspect("order").user("ujoy").mode(eco::log::mode_rsp)
        << "buy 100 shares of AAPL failed";
    // [info ] (...) <trader.order> ::: @ujoy buy 100 shares of AAPL. (each=93/s)
    int rc = -1;
    trader_log(info, rc == 0, 1000, 0).aspect("order").user("ujoy")
        .format("buy {} shares of AAPL (each={}/s)", 100, each.frequency());
    
    // [debug] (...) <market> ::: 
    //market_log(debug);
    // [info ] (...) <target> ::: 
    //target_log(info);
    // [warn ] (...) <settle> ::: 
    //settle_log(warn);
    // [error] (...) <riskctrl> ::: 
    //riskctrl_log(error);
}