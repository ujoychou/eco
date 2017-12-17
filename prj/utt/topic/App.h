#ifndef ECO_CMD_COMMAND_APP_H
#define ECO_CMD_COMMAND_APP_H
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
#include <eco/thread/topic/TopicServer.h>
#include "Test.h"


namespace eco{;
namespace topic{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class App : public eco::App
{
public:
	App();
	virtual void on_init() override;
	virtual void on_exit() override;

private:
	Subscriber m_suber;
	eco::TopicServer m_topic_server;
};

ECO_APP(App, GetApp);
}}}
#endif