#include <eco/app.hpp>
////////////////////////////////////////////////////////////////////////////////
#include <functional>
#include "HelloWorld.hpp"


////////////////////////////////////////////////////////////////////////////////
class HelloWorldService : public eco::service
{
public:
    void SayHello(eco::session_request<proto3::HelloWorldReq>& sess)
    {
        proto3::HelloWorldRsp rsp;
        //rsp.set_name(sess.request().name());
        if (rsp.name().empty())
        {
            ECO_THROW(1001);
        }
        if (rsp.name().find("hello"))
        {
            ECO_THROW(1002) << rsp.name();
        }
        sess.reply(rsp);
    }
};


////////////////////////////////////////////////////////////////////////////////
class HelloWorldServer : eco::app
{
public:
	virtual void on_init()
	{
		this->helloworld = eco::app::service<HelloWorldService>(HelloWorld_ID::service);
		this->helloworld.encoder<eco::encoder::json>();
		this->helloworld.encrypt<eco::encrypt::being>();
		this->helloworld.protocol<eco::protocol::mqtt>();
		this->helloworld.remote().encoder<eco::encoder::protobuf3>();
		this->helloworld.remote().protocol<eco::protocol::grpc>();

		// service method
		using namespace std::placeholders;
		this->helloworld.method(HelloWorld_ID::say_hello).bind<
			proto3::HelloWorldReq>(
			std::bind(&HelloWorldService::SayHello, &helloworld, _1));
	}

public:
	HelloWorldService helloworld;
};


ECO_APP(HelloWorldServer);
////////////////////////////////////////////////////////////////////////////////