#include "market_defs.hpp"
#include <string>


class RealtimeService
{
public:
    void get_tick(market::tick& tick, const std::string& name)
    {
        // 支持多语言
        ECO_THROW(-10001)(
            "%s are the best logging %d", name.c_str(), tick.d);
    }

    void get_product(market::product& product, uint64_t id, const std::string& name)
    {
        test += "100";
        // 支持多语言
        ECO_THROW(-10001)(
            "%s are the best logging %d", name.c_str(), tick.d);
    }

private:
    std::string test;
};


////////////////////////////////////////////////////////////////////////////////
class MarketApp : public eco::app
{
public:
    void MarketApp::on_init() override
    {
        // 配置信息
        eco::app::config().file("market_front.json");
        eco::app::config().file("market_front.xml");
        // 路由服务：服务发现
        eco::app::router().ip("127.0.0.1").port(45535);
        // 监控信息
        eco::app::monitor().domain(domain_monitor).service(service_monitor);
    }

    // 无端化服务架构

    // 调用服务
    void MarketApp::on_init() override
    {
        // 实时服务
        realtime = eco::app::service(domain_market, service_realtime);
        realtime->method(method_get_tick);
        realtime->method(method_get_target);
        realtime->set_protocol(eco::protocol::someip_v3);

        // 服务调用：历史服务
        history = eco::app::service(domain_market, service_history);
        realtime->method(method_get_tick);
        realtime->method(method_get_target);
        realtime->set_protocol(eco::protocol::rtps_v1, "168.192.10.1:9597");
        realtime->set_protocol(eco::protocol::rtps_v2, "168.192.10.2:9597");
    }

    // 提供服务
    void MarketApp::on_init() override
    {
        // 服务调用：实时服务
        using std::placeholder;
        realtime = eco::app::service(domain_market, service_realtime);
        service.get_tick = realtime->create_method(method_get_tick, 
            std::bind(&RealtimeService::get_tick, &service, _1, _2));
        service.get_product = realtime->create_method(method_get_target,
            std::bind(&RealtimeService::get_product, &service, _1, _2, _3));
        realtime->protocol(eco::protocol::someip_v3);

        // 服务调用：历史服务
        //history = eco::app::service(domain_market, service_history);
        service.get_tick = realtime->method(method_get_tick).bind(
            std::bind(&RealtimeService::get_tick &service, _1, _2));
        service.get_product = realtime->method(method_get_target);
        realtime->protocol_remote(eco::protocol::rtps_v1, "168.192.10.1:9597");
        realtime->protocol_remote(eco::protocol::rtps_v2, "168.192.10.2:9597");
        realtime->protocol(eco::protocol::someip_v2);
        realtime->protocol(eco::protocol::someip_v3);
        realtime->method(method_get_tick).sync();
        realtime->method(method_get_tick).async();
        realtime->serialize_remote(eco::serialize::protobuf3, "168.192.10.1:9597");
        realtime->serialize(eco::serialize::protobuf3, true);
        realtime->serialize(eco::codec::xcdr1);
        realtime->serialize(eco::codec::xcdr2);
        realtime->serialize(eco::codec::xcdr3, default: true);
        realtime->serialize(eco::codec::protobuf2, false);
        realtime->serialize(eco::codec::protobuf3, true);

        // 序列化
        protobuf::Request   req;
        protobuf::Response  rsp;
        protobuf::Error     err;
        int ec = realtime->method(method_get_tick).sync(&req, &rsp);
        if (ec)
        {
            ECO_THROW();
        }
        

        realtime->method(method_get_tick).async(&req,
            [](protobuf::Response& rsp, protobuf::Error* err, int sys) {

        });

        
    }

    // call method
    inline void call()
    {
        int c = service.get_tick(a, b);
        int c = service.get_tick(a, b).async([](){

        });
    }

private:
    eco::service::ref realtime;     // 实时行情
    eco::service::ref history;      // 

    // realtime method
    Realtime realtime_proxy;
    RealtimeService  service;
};