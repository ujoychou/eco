#ifndef XXX_APP_H
#define XXX_APP_H
/*******************************************************************************
@ 名称

@ 功能

@ 异常

@ 备注

--------------------------------------------------------------------------------
@ 历史记录 @
@ ujoy modifyed on 2016-10-29

--------------------------------------------------------------------------------
* 版权所有(c) 2015 - 2017, ujoychou, 保留所有权利。

*******************************************************************************/
#include <eco/App.h>
#include <eco/proto/Test.pb.h>

namespace xxx{ ;


////////////////////////////////////////////////////////////////////////////////
class App : public eco::App
{
public:
	// 构造函数，可初始化"sys"日志文件。
	App();

protected:
	// 初始化APP
	virtual void on_init() override;
	virtual void on_load() override;

	// 退出APP
	virtual void on_exit() override;

	// 默认
	void on_default(IN eco::net::Context& c);
	void on_rsp(IN proto::TestRsp& rsp, IN eco::net::Context& c);
	void on_run();

private:
	eco::net::TcpClient m_client;
	eco::Thread m_thread;
	int m_req_id;
};


ECO_APP(App, get_app);
////////////////////////////////////////////////////////////////////////////////
}
#endif