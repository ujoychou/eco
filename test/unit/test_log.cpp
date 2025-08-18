#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/log.hpp>


////////////////////////////////////////////////////////////////////////////////
class test_log : public ::testing::Test
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
        eco_log(debug).title("FUNC").format("a=% < b=%") % a % b << c;
        eco_log(debug).title("FLAG").printf("a=%d < b=%d", a, b) << c;
        eco_log(debug).title("MODU") << "a=" << a << " < b=" << b << c;
    }

    inline void log_cond()
    {
        int a = 0, b = 1;

        // when: condition
        /*eco_log(debug, a < b).format("a=% < b=%") % a % b;
        eco_log(debug, a < b).printf("a=%d < b=%d", a, b);
        eco_log(debug, a < b) << "a=" << a << " < b=" << b;
        eco_log(debug, a < b);*/

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

    inline void log_fatal()
    {
    }
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(test_log, elog)
{
    // logger1
    eco::log::logger::config c;
    c.name = "console";
    c.type = "eco_console";
    c.level_min = eco::log::warn;    
    eco::log::elog::logger(c);
    // logger2
    c.name = "logging";
    c.type = "eco_logging";
    c.level_min = eco::log::info;
    eco::log::elog::logger(c);

    // config
    eco::log::config conf;
    conf.level_min = eco::log::info;
    conf.cache_size = 4 * 1024 * 1024;
    conf.entry_size = 1024;
    conf.format_logger_type = "eco_logging";
    eco::log::elog::start(conf);

    // logging test
    log_level();
    log_format();
    log_cond();
    log_fatal();
}