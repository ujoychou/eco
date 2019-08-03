#include "PrecHeader.h"
#include "App.h"
////////////////////////////////////////////////////////////////////////////////


namespace xxx{;
////////////////////////////////////////////////////////////////////////////////
App::App()
{
	eco::App::set_sys_config_file("config/sys.xml");
}


////////////////////////////////////////////////////////////////////////////////
void App::on_init()
{
	m_req_id = 0;
	m_client = eco::App::get_consumer();
	m_client.register_default_handler(std::bind(
		&App::on_default, this, std::placeholders::_1));
	m_client = eco::App::get_consumer();
	m_client.dispatch<proto::TestRsp>(2, *this, &App::on_rsp);
}
void App::on_load()
{
	m_thread.run(std::bind(&App::on_run, this), "test_run");
}


////////////////////////////////////////////////////////////////////////////////
void App::on_exit()
{
}
void App::on_default(IN eco::net::Context& c)
{
	//EcoError << "unknown message.";
}
void App::on_rsp(IN proto::TestRsp& rsp, IN eco::net::Context& c)
{
	EcoInfo << "rsp=" <= rsp.data().int1();
}


////////////////////////////////////////////////////////////////////////////////
void App::on_run()
{
	proto::TestReq req;
	req.set_int1(1);
	req.set_int1(2);
	req.set_int1(3);
	req.set_int1(4);
	req.set_str1("string1");
	req.set_str2("string2");
	req.set_str3("string3");
	req.set_str4("string4");
	req.set_dbl1(1.0);
	req.set_dbl1(2.0);
	req.set_dbl1(3.0);
	req.set_dbl1(4.0);
	proto::TestRsp rsp;
	while (true)
	{
		req.set_int1(++m_req_id);
		m_client.send(req, 1, true);
		EcoInfo << "req" <= req.int1();
		eco::this_thread::sleep(100);
	}
}



////////////////////////////////////////////////////////////////////////////////
}