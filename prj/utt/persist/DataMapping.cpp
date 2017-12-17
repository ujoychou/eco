#include "PrecHeader.h"
#include "DataMapping.h"
////////////////////////////////////////////////////////////////////////////////


namespace eco{;
namespace persist{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
void DataMapping::Init()
{
	// ORM映射关系配置。
	// 股票
	m_stock_orm.id("stock_mapping").table("stock");
	m_stock_orm.add().property("id").field("id").pk(true).varchar(16);
	m_stock_orm.add().property("name").field("name").varchar(32);
	m_stock_orm.add().property("group_id").field("group_id").big_int();

	// 股票组
	m_stock_group_orm.id("stock_group_mapping").table("stock_group");
	m_stock_group_orm.add().property("id").field("id").pk(true).big_int();
	m_stock_group_orm.add().property("name").field("name").varchar(32);
	m_stock_group_orm.add().property("pool_id").field("pool_id").big_int();

	// 股票池
	m_stock_pool_orm.id("stock_pool_mapping").table("stock_pool");
	m_stock_pool_orm.add().property("id").field("id").pk(true).big_int();
	m_stock_pool_orm.add().property("name").field("name").varchar(32);
}


////////////////////////////////////////////////////////////////////////////////
}}}// ns