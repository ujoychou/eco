#include "PrecHeader.h"
#include "App.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include "Database.h"


namespace eco{;
namespace persist{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
App::App()
{
	// 初始化数据库.
	GetDataMapping().Init();
	m_database.reset(new Database("vision.db"));

	// 1.插入新数据
	std::shared_ptr<Stock> stock(new Stock);
	stock->m_id = "600018";
	stock->m_name = "招商银行";
	stock->m_group_id = 10001;
	m_database->Source().save(*stock, StockMeta(), GetDataMapping().GetStock());
	// 1.插入新数据（方式1）
	stock.reset(new Stock);
	stock->m_id = "600058";
	stock->m_name = "民生银行";
	stock->m_group_id = 100001;
	m_database->Source().save(*stock, StockMeta(), GetDataMapping().GetStock());
	// 1.插入新数据（方式2）
	stock->m_id = "600048";
	stock->m_name = "贵州茅台";
	stock->m_group_id = 10002;
	stock->timestamp().insert();	// 指定插入操作
	m_database->Source().save(*stock, StockMeta(), GetDataMapping().GetStock());

	// 读取指定数据
	stock.reset(new Stock);
	stock->m_id = "600018";
	m_database->Source().read(*stock, StockMeta(), GetDataMapping().GetStock());

	//--------------------------------------------------------------------------
	// 2.更新数据
	stock->SetValue("name", "五粮液");
	m_database->Source().save(*stock, StockMeta(), GetDataMapping().GetStock());

	// 读取更新后的数据
	stock.reset(new Stock);
	stock->m_id = "600018";
	m_database->Source().read(*stock, StockMeta(), GetDataMapping().GetStock());

	// 查询所有数据
	std::list<Stock::value> stock_list;
	m_database->Source().select(
		stock_list, StockMeta(), GetDataMapping().GetStock());

	// 查询指定数据
	stock_list.clear();
	m_database->Source().select(
		stock_list, StockMeta(), GetDataMapping().GetStock(),
		"id like '600%'");

	//--------------------------------------------------------------------------
	// 3.删除指定数据
	stock.reset(new Stock);
	stock->m_id = "600018";
	stock->timestamp().remove();
	m_database->Source().save(*stock, StockMeta(), GetDataMapping().GetStock());

	// 查询所有数据
	stock_list.clear();
	m_database->Source().select(
		stock_list, StockMeta(), GetDataMapping().GetStock());

	// 删除所有数据
	m_database->Source().remove(
		GetDataMapping().GetStock().get_table());

	// 查询所有数据
	stock_list.clear();
	m_database->Source().select(
		stock_list, StockMeta(), GetDataMapping().GetStock());
}


////////////////////////////////////////////////////////////////////////////////
}}}