#ifndef ECO_PERSIST_APP_H
#define ECO_PERSIST_APP_H
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
#include "Database.h"


namespace eco{;
namespace persist{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class App : public eco::App
{
public:
	App();

private:
	std::shared_ptr<Database> m_database;
};

ECO_APP(App, GetApp);
}}}
#endif