#include <eco/app.hpp>
////////////////////////////////////////////////////////////////////////////////
#include "HelloWorld.hpp"


////////////////////////////////////////////////////////////////////////////////
class HelloWorldClient : eco::app
{
public:
	virtual void on_init()
	{
		this->helloworld = eco::app::service(HelloWorld_ID::service);
		this->helloworld.encrypt<eco::encrypt::being>();
		this->helloworld.encoder<eco::encoder::protobuf3>();
		this->helloworld.protocol<eco::protocol::grpc>();
		this->helloworld.remote().encoder<eco::encoder::json>();
		this->helloworld.remote().protocol<eco::protocol::mqtt>();
	}

	virtual void on_load()
	{
		proto3::HelloWorldReq req;
		proto3::HelloWorldRsp rsp;
		this->helloworld.method(HelloWorld_ID::say_hello).sync(req, rsp);
		ECO_INFO("say hello to %s reply back.", rsp.name().c_str());

		this->helloworld.method(HelloWorld_ID::say_hello).async<
			proto3::HelloWorldRsp>(req, 
			[&](eco::session_reply<proto3::HelloWorldRsp>& sess) {
			if (sess.has_error())
			{
				ECO_ERROR() << ECO_THIS_ERROR();
			}
			ECO_LOG(info, "async say hello to %s reply back.", 
				sess.reply().name().c_str());
		});


		const char* name = "sssssdddsss";
		ECO_INFO() << eco::gr(name) << eco::sq(name) << eco::br(name) << eco::sp(name);
		ECO_INFO() << eco::group(name) << eco::square(name) << eco::brace(name) << eco::space(name)
		<< eco::upper(eco::boolean(true, 1)) << eco::percent(name) << eco::brace(name) << eco::space(name);
	}

public:
	eco::service helloworld;
};


ECO_APP(HelloWorldClient);
////////////////////////////////////////////////////////////////////////////////