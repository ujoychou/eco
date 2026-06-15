#ifndef APP_FRAMEWORK_HPP
#define APP_FRAMEWORK_HPP
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ history v1.0
[ujoy 2024-06-29]
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <memory>
#include <chrono>
#include <thread>
#include <string>


namespace eco {
////////////////////////////////////////////////////////////////////////////////
// 作用：类似于DDS的Topic，负责订阅发布通信。
class topic
{
public:
    typedef std::shared_ptr<topic> ref;
};

class topic_reader
{
public:
    typedef std::shared_ptr<topic_reader> ref;

};

class topic_writer
{
public:
    typedef std::shared_ptr<topic_writer> ref;
};

class topic_reader_qos
{
public:
    typedef std::shared_ptr<topic_reader> ref;

};

class topic_writer_qos
{
public:
    typedef std::shared_ptr<topic_writer> ref;
};

class topic_reader_listener
{
public:
    typedef std::shared_ptr<topic_reader> ref;

};

class topic_writer_listener
{
public:
    typedef std::shared_ptr<topic_writer> ref;
};

class method
{
public:
    typedef std::shared_ptr<method> ref;

};

////////////////////////////////////////////////////////////////////////////////
// 作用：通信的基本单位，定义服务信息。
class service
{
public:
    typedef std::shared_ptr<service> ref;

    template<typename object_t>
    void subscribe_object();

    topic::ref topic(uint64_t id);
    topic::ref topic(const char* name);

    topic_reader::ref topic_reader(
        uint64_t id);
    topic_reader::ref topic_reader(
        const char* name, int create_writer);

    topic_writer::ref topic_writer(
        uint64_t id);
    topic_writer::ref topic_writer(
        const char* name, int create_service_topic);
    topic_writer::ref topic_writer_find(
        uint64_t id);
    topic_writer::ref topic_writer_find(
        const char* name, int create_service_topic);

public:
    // 轻松实现RPC服务
    method::ref method(int32_t method_id)
    {
        discovery(method_id);
    }

    // 轻松实现RPC服务
    template<typename func_t>
    method::ref method(int32_t method_id, func_t&& impl)
    {
        provide(method_id);
    }
};


////////////////////////////////////////////////////////////////////////////////
// 作用：隔离服务
class domain
{
public:
    typedef std::shared_ptr<domain> ref;

    static domain::ref get(int id, const char* name = "");
    service::ref service(int id, const char* name = "");
};

// 作用：提供数据通道
class server
{
public:
    typedef std::shared_ptr<server> ref;
    static ref get();

    void add_service(service::ref& service);

    template<typename codec_t>
    void add_codec();

    template<typename protocol_t>
    void add_protocol();

    template<typename transport_t>
    void add_transport();
};

////////////////////////////////////////////////////////////////////////////////
namespace conf
{
    class monitor
    {
    public:
        monitor& domain(int value);
        monitor& service(int value);
    };

    class logging
    {
    public:
        logging& domain(int value);
        logging& service(int value);
    };
};


// 作用：管理程序的生命周期，一个进程一个app单实例
class app
{
public:
    virtual void on_init() {}
    virtual void on_cmd()  {}
    virtual void on_load() {}
    virtual void on_exit() {}

public:
    // 
    eco::conf::logging& logging();
    eco::conf::monitor& monitor();

    // 
    server::ref server();
    service::ref service(
        int domain,
        int service,
        const char* name);
};


class Student {};
namespace proto {
    class Student;
    class StudentMeta;
};


void subscribe_test()
{
    service::ref service;
    uint64_t student_key = 20250518;
    service->subscribe_object<proto::Student, proto::StudentMeta>(student_key, 
        [](const char* name, const char* value) {
            ObjectMeta(obj).set_property(name, value);
            callback(obj);
        });
    service->subscribe_object<proto::Student, proto::StudentMeta>(student_key, 
        [](std::shared_ptr<proto::Student>& obj) {});
}

class object_change
{
    data_t*  data;
    uint32_t property_id;
    char*    value;
    uint32_t value_size;
};



////////////////////////////////////////////////////////////////////////////////
} // eco
#endif

