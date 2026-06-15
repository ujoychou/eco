#include "app_framework.hpp"



////////////////////////////////////////////////////////////////////////////////
int topic_service()
{
    // 座舱域
    //eco::App app = eco::create_app("vehicle");
    eco::domain dom_cabin = eco::create_domain("cabin");
    eco::service svr_map = dom_cabin.create_service("map");
    eco::topic topic_loc = svr_map.create_topic("location");
    eco::topic_writer_qos wqos;
    eco::topic_reader_qos rqos;
    eco::TopicReaderListener wlistener;
    eco::TopicReaderListener rlistener;
    eco::TopicWriter writer_loc = topic_loc.create_writer(qos);
    eco::TopicReader reader_loc = topic_loc.create_Reader(qos);
    eco::Topic topic_video_f1 = svr_map.create_topic("video_f1");
    eco::Topic topic_video_f2 = svr_map.create_topic("video_f2");

    // 行情系统：实时行情系统
    //eco::App app = eco::create_app("realtime");
    eco::Domain dom = eco::create_domain("market");
    eco::Service svc = dom->create_service("tick");
    eco::Service svc_history = dom.create_service("history");
    eco::Service svc_realtime = dom.create_service("realtime");
    eco::Topic top_tick = svc->create_topic("tick", meta::CodecMarketTick);
    eco::Topic top_1min = svc->create_topic("1min", meta::CodecMarketBar);
    eco::TopicWriterQos wqos_ys;
    eco::TopicWriterQos wqos_zd;
    wqos_ys.ownership().strength(5);
    wqos_zd.ownership().strength(3);
    eco::TopicWriter wr_ys = topic.create_writer(wqos_ys);
    eco::TopicWriter wr_zd = topic.create_writer(wqos_zd);
    MarketTick tick;
    wr_ys.write(tick);
    wr_zd.write(tick);

    // 服务Handler处理
    enum
    {
        get_tick = 1,
        get_bar1 = 2,
        get_bar2 = 2,
    };
    typedef int MarketTickReq;
    typedef int MarketTick;
    typedef int MarketBarReq;
    typedef int MarketBar;
    eco::Method method;
    method = svc->create_method(get_tick, [](eco::net::Request<MarketTickReq>& req) {
        MarketTick rsp;
        req.response<MarketTick>(rsp);
    });
    method = svc->create_method(get_bar1, [](eco::net::Request<MarketBarReq>& req) {
        MarketBar rsp;
        req.response<MarketBar>(rsp);
    });
    method = svc->create_method(get_bar2, [](eco::net::Request<MarketBarReq>& req) {
        MarketBar bar;
    });
    top_1min->publish<MarketBar>(bar);

    svc->provide(get_bar1, [](eco::net::Request<MarketBarReq>&){});
    svc->request(get_bar1, MarketBarReq(),
        [](eco::net::Request<MarketBarReq>&){},
        [](eco::net::Error&){});

    class method : public task
    {}

    eco::method method = svc->provide(get_bar1);
    method.async<method_request_t>();
    method.async(std::function<>());

    eco::method method = svc->request(get_bar1, MarketBarReq());
    method.async<method_reply_t>();
    method.async(std::function<>());

    // 服务提供者
    eco::Provider provider = eco::create_provider();
    provider.add_codec<eco::codec::Xcdr1>();
    provider.add_codec<eco::codec::Xcdr2>();
    provider.add_protocol<eco::ap::Rtps1>();
    provider.add_protocol<eco::ap::Rtps2>();
    provider.add_transport<eco::tp::Udp>();
    provider.add_transport<eco::tp::Shm>();
    provider.add_transport<eco::tp::ShmZcopy>();
    provider.add_service(svc);
    provider.add_service(svc_history);
    provider.add_service(svc_realtime);

    // 行情系统：实时行情系统

    return 0;
}


typedef int MarketTick;
class MarketProxy
{
public:
    void on_tick();
    void on_kbar();
    void on_kbar_min_1();

};

////////////////////////////////////////////////////////////////////////////////
int topic_service_roxy()
{
    // 行情系统：实时行情系统
    //eco::App app = eco::create_app("realtime");
    eco::Domain dom = eco::create_domain("market");
    eco::ServiceProxy pxy_tick = dom.create_service_proxy("tick");
    eco::ServiceProxy pxy_tick = dom.create_service_proxy("history");
    eco::ServiceProxy pxy_tick = dom.create_service_proxy("realtime");
    eco::Topic top_tick = pxy_tick->create_topic("tick", meta::CodecMarketTick);
    eco::Topic top_1min = pxy_tick->create_topic("1min", meta::CodecMarketBar);
    eco::TopicReaderQos rqos;
    eco::TopicReaderListener rlistener = topic.create_reader_listener<MarketTick>(
        std::bind()
    );
    eco::TopicReader reader = topic.create_reader(rqos, rlistener);
    eco::TopicReaderListener rlistener = reader.create_listener();

    MarketTick tick;
    reader_ys.listener(tick);
    reader_ys.write(tick);
}