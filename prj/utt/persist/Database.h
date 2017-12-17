#ifndef ECO_PERSIST_TEST_H
#define ECO_PERSIST_TEST_H
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
#include <eco/thread/ThreadPool.h>
#include <eco/persist/Sqlite.h>
#include "Data.h"
#include "DataMapping.h"


namespace eco{;
namespace persist{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class Database
{
public:
	Database(IN const std::string& file);

	// 读取股票
	void ReadStock(
		OUT Stock& stock,
		IN  const std::string& stock_id);

	// 读取股票
	void ReadSomeStock(
		OUT Stock& stock,
		IN  const std::string& stock_id);

	// 数据源
	inline eco::Database& Source()
	{
		return *m_source;
	}

private:
	// 创建表格
	void InitTable();

private:
	std::shared_ptr<eco::Sqlite> m_source;
};


}}}
#endif