////////////////////////////////////////////////////////////////////////////////
#include <eco/plugin.hpp>
#include <eco/rtti/app.hpp>
#include <eco/log.hpp>


using namespace eco::log;
////////////////////////////////////////////////////////////////////////////////
class test_glog : public eco::log::logger
{
    eco_plugin(test_glog, "1.0.0");
public:
    void on_entry_format(message& message, on_time on) override
    {
        if (on == on_begin)
           message.entry.append("[GLOG_1.0.0] <BEGIN>");
        else if (on == on_end)
            message.entry.append("<END>");
    }

    void on_entry_output(message& message) override
    {
        message.entry.append(" OUTPUT");
    }
};


////////////////////////////////////////////////////////////////////////////////
class test_blog : public eco::log::logger
{
    eco_plugin(test_blog, "2.1.0");
public:
    void on_entry_format(message& message, on_time on) override
    {
        if (on == on_begin)
            message.entry.append("[BLOG_2.1.0] <BEGIN>");
        else if (on == on_end)
            message.entry.append("<END>");
    }

    void on_entry_output(message& message) override
    {
        message.entry.append(" OUTPUT");
    }
};


////////////////////////////////////////////////////////////////////////////////
class test_clog : public eco::log::logger
{
    eco_plugin(test_clog, "2.2.0");
public:
    void on_entry_format(message& message, on_time on) override
    {
        if (on == on_begin)
            message.entry.append("[CLOG_2.2.0] <BEGIN>");
        else if (on == on_end)
            message.entry.append("<END>");
    }

    void on_entry_output(message& message) override
    {
        message.entry.append(" OUTPUT");
    }
};


////////////////////////////////////////////////////////////////////////////////
class LogRxApp : public eco::rtti::app
{
public:
    eco::result on_init() override
    {
        // init glog
        return eco::result::ok;
    }

    eco::result on_exit() override
    {
        // exit glog
        return eco::result::ok;
    }
};
eco_erx(LogRxApp, erx_app);