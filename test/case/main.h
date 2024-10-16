
#include "TcpServer.h"

// tcp connect
class TcpConnect : public eco::net::TcpConnect
{
public:
};


class Handler
{
public:
    using SessionLogin = TReqSession<LoginReq, LoginRsp, ProtobufCodec, REQ_OPT_INIT>;
    static void login(SessionLogin* login)
    {
    }

    using SessionLogout = TReqSession<LoginReq, LoginRsp, ProtobufCodec, REQ_OPT_INIT>;
    static void logout(SessionLogout* logout)
    {
    }
};

// tcp server
// 特点：异步，简洁
eco::net::TcpServer& provider();
void uc_server()
{
    // connect扩展方式
    provider().set_connect<TcpConnect>();
    provider().set_address(eco::net::Address);

    // session其实并不存在，他只是实现这个接口的基础设置
    // 如果c++模板技术更好，则不需要这个参数：handler::login(SessionLogin*)
    // provider().dispatch(10004, handler::login);
    provider().dispatch<SessionLogin>(10004, handler::login);
    provider().dispatch<SessionLogout>(10005, handler::logout);
    provider().dispatch<SessionIer>(10006, handler::Ier);
    provider().dispatch<SessionOcr>(10007, handler::Ocr);
    provider().start();
}


void uc_client()
{
    
}


void uc_task_server()
{
    
}

void uc_task_client()
{
    
}

void uc_task_general()
{
    
}


class Message
{
public:
    void append(const char* buff, int size)
    {

    }

    void 
};


class Family
{
    void encode(const char* buff);
    void decode(const char* buff);
};


////////////////////////////////////////////////////////////////////////////////
/* 基本需求
1. 应用层与传输层：职责分离。
2. 序列化与协议：职责分离。
3. 安全传输协议：可配置。
4. 应用层协议AP：增加长度字段，从而同时支持UDP与TCP。（不支持）
（为了性能的最优化，不同时支持UDP与TCP）
5. 应用层协议AP：应用TCP时，应去除掉应对UDP的相关处理。（不支持）
6. 传输层协议TP：SHM与SHMZCOPY的区别是后者不需要处理序列化。
7. 服务可以定义多个：从而自动实现负载均衡，Topic依赖于服务而存在。
*/


////////////////////////////////////////////////////////////////////////////////
namespace eco {;
// 应用层
class Domain {};
class Service {};
class Method {};

class Topic {};
class TopicWriter {};
class TopicReader {};
class TopicWriterQos {};
class TopicReaderQos {};
class TopicWriterListener {};
class TopicReaderListener {
    virtual void on_event_1();
    virtual void on_event_2();
};

class Provider {};
class Consumer {};

// 序列化
namespace codec {;
class Xml {};
class Xcdr1 {};
class Xcdr2 {};
class Someip3 {};
class Json {};
class Protobuf3 {};
class Ftdcodec {};
class Codec
{    
    virtual void encode() = 0;
    virtual void decode() = 0;
    virtual size_t encode_size() = 0;
    virtual size_t decode_size() = 0;
};
class Crypt
{    
    virtual void encode() = 0;
    virtual void decode() = 0;
    virtual size_t encode_size() = 0;
    virtual size_t decode_size() = 0;
};
}

// 应用层协议
namespace ap {;
class Grpc {};
class Srpc {};
class Brpc {};
class Someip {};
class Rtps {};
class Ftdc {};
class Kafka {};
class Mysql {};
class Http {};
class Json {};

class Rtps1 {};
class Rtps2 {};

class Protocol
{
    virtual void parse() = 0;
    virtual void build() = 0;
    virtual size_t parse_size() = 0;
    virtual size_t build_size() = 0;
};
}

// 传输层协议
namespace tp {;
class Tcp {};
class Udp {};
class Shm {};
class Zcc {};
class Quic {};

class Tsl {};
class Ssl {};
}

}


////////////////////////////////////////////////////////////////////////////////
int topic_service()
{
    // 座舱域
    //eco::App app = eco::create_app("vehicle");
    eco::Domain dom_cabin = eco::create_domain("cabin");
    eco::Service svr_map = dom_cabin.create_service("map");
    eco::Topic topic_loc = svr_map.create_topic("location");
    eco::TopicWriterQos wqos;
    eco::TopicReaderQos rqos;
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
////////////////////////////////////////////////////////////////////////////////