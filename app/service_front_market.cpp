#include "market_defs.hpp"


////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
class MarketFrontApp : public eco::app
{
public:
    void MarketFrontApp::on_init() override
    {
        // 配置信息
        eco::app::config().file("market_front.json");
        eco::app::config().file("market_front.xml");
        // 路由服务：服务发现
        eco::app::router().ip("127.0.0.1").port(45535);
        
        // 监控信息
        eco::app::monitor().domain(domain_monitor).service(service_monitor);
    }

    void MarketFrontApp::on_init() override
    {
        // 服务信息
        realtime = eco::app::service(domain_market, service_realtime);
        history = eco::app::service(domain_market, service_history);
        server = eco::app::server();
        server->add_codec<eco::codec::Xcdr1>();
        server->add_codec<eco::codec::Xcdr2>();
        server->add_protocol<eco::ap::Rtps1>();
        server->add_protocol<eco::ap::Rtps2>();
        server->add_transport<eco::tp::Udp>();
        server->add_transport<eco::tp::Shm>();
        server->add_transport<eco::tp::ShmZcopy>();
        server->add_service(realtime);
        server->add_service(history);

        // 订阅主题
        eco::server
    }

private:
    eco::server::ref  server;
    eco::service::ref realtime;     // 实时行情
    eco::service::ref history;      // 
};