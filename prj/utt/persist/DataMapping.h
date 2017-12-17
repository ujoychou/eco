#ifndef ECO_PERSIST_DATA_MAPPING_H
#define ECO_PERSIST_DATA_MAPPING_H
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
#include <eco/persist/ObjectMapping.h>
#include <eco/meta/Timestamp.h>


namespace eco{;
namespace persist{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
// 持久化映射关系
class DataMapping
{
	ECO_SINGLETON(DataMapping);
public:
	void Init();

	// 股票
	inline const eco::ObjectMapping& GetStock() const
	{
		return m_stock_orm;
	}

	// 股票组
	inline const eco::ObjectMapping& GetStockGroup() const
	{
		return m_stock_group_orm;
	}

	// 股票池
	inline const eco::ObjectMapping& GetStockPool() const
	{
		return m_stock_pool_orm;
	}

private:
	eco::ObjectMapping m_stock_orm;
	eco::ObjectMapping m_stock_group_orm;
	eco::ObjectMapping m_stock_pool_orm;
};
ECO_SINGLETON_GET(DataMapping);


}}}
#endif