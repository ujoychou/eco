#include "PrecHeader.h"
#include "Database.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/Error.h>



namespace eco{;
namespace persist{;
namespace test{;



////////////////////////////////////////////////////////////////////////////////
Database::Database(IN const std::string& file)
{
	// 打开文件
	m_source.reset(new eco::Sqlite());
	m_source->open(file.c_str());

	// 创建表格
	InitTable();
}


////////////////////////////////////////////////////////////////////////////////
void Database::InitTable()
{
	std::string sql;
	if (!m_source->has_table(GetDataMapping().GetStock().get_table()))
	{
		GetDataMapping().GetStock().get_create_table_sql(sql);
		m_source->execute_sql(sql.c_str());
	}
	if (!m_source->has_table(GetDataMapping().GetStockGroup().get_table()))
	{
		GetDataMapping().GetStockGroup().get_create_table_sql(sql);
		m_source->execute_sql(sql.c_str());
	}
	if (!m_source->has_table(GetDataMapping().GetStockPool().get_table()))
	{
		GetDataMapping().GetStockPool().get_create_table_sql(sql);
		m_source->execute_sql(sql.c_str());
	}
}


////////////////////////////////////////////////////////////////////////////////
}}}