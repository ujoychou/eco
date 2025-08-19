////////////////////////////////////////////////////////////////////////////////
#include <eco/plugin.hpp>
#include <eco/rtti/app.hpp>
#include <eco/log.hpp>


using eco::log;
thread_local 
////////////////////////////////////////////////////////////////////////////////
class test_glog : public eco::log::logger
{
    eco_plugin("glog", "1.0.0");
public:
    void on_entry_format(message& message, on_time on) override
    {
        if (on == on_begin)
           message.entry().append("[GLOG_1.0.0] <BEGIN>");
        else if (on == on_end)
            message.entry().append("<END>");
    }

    void on_entry_output(message& message) override
    {
        message.entry().append(" OUTPUT");
    }
};


////////////////////////////////////////////////////////////////////////////////
class test_blog : public eco::log::logger
{
    eco_plugin("blog", "2.1.0");
public:
    void on_entry_format(message& message, on_time on) override
    {
        if (on == on_begin)
            message.entry().append("[BLOG_2.1.0] <BEGIN>");
        else if (on == on_end)
            message.entry().append("<END>");
    }

    void on_entry_output(message& message) override
    {
        message.entry().append(" OUTPUT");
    }
};


////////////////////////////////////////////////////////////////////////////////
class test_clog : public eco::log::logger
{
    eco_plugin("blog", "2.2.0");
public:
    void on_entry_format(message& message, on_time on) override
    {
        if (on == on_begin)
            message.entry().append("[CLOG_2.2.0] <BEGIN>");
        else if (on == on_end)
            message.entry().append("<END>");
    }

    void on_entry_output(message& message) override
    {
        message.entry().append(" OUTPUT");
    }
};


////////////////////////////////////////////////////////////////////////////////
class LogRxApp : public eco::rx::app
{
public:
    void on_init() override
    {
        // init glog
    }

    void on_exit() override
    {
        // exit glog
    }
};
eco_erx(LogRxApp, erx_app);