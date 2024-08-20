#include "app_framework.hpp"
////////////////////////////////////////////////////////////////////////////////
#include "market_defs.hpp"





////////////////////////////////////////////////////////////////////////////////
class market_view_flash
{
public:
    eco::topic_reader target;
    eco::topic_reader bar_1min;

    void sub_bar()
    {
        bar_1min.close();
        this->server->history->reader(target);
    }

public:
    market_server* server;
};

class market_view_tick
{
public:
    eco::topic_reader target;
    
};


////////////////////////////////////////////////////////////////////////////////
// 行情客户端：APP
class market_client
{
public:
    inline market_client()
    {
        dom = eco::domain::get(domain_market, "market");
        realtime = dom->service(service_realtime, "realtime");
        history = dom->service(service_history, "history");

        // 实时行情
    }

    // 订阅行情
    inline void sub_tick(const char* target)
    {
        realtime->topic_reader(target, true);
    }

    // 取消订阅
    inline void unsub_tick(const char* target)
    {
        realtime->topic_reader(target, true);
    }

public:
    // 开始订阅
    inline void start()
    {
        this->sub_tick("IF2408");
        this->sub_tick("GC2408");
        this->sub_tick("MGC2408");

        // 等待2秒后，重新订阅合约数据
        std::this_thread::sleep_for(std::chrono::seconds(2));
        this->unsub_tick("IF2408");
        this->unsub_tick("GC2408");
        this->unsub_tick("MGC2408");
        this->sub_tick("IF2409");
        this->sub_tick("GC2409");
        this->sub_tick("MGC2409");
    }

public:
    eco::domain::ref dom;
    eco::service::ref realtime;
    eco::service::ref history;

    // view: flash graph
    eco::topic_reader target_r_flash;
    // view: tick graph
    eco::topic_reader target_r_tick;
};