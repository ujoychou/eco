#include "Pch.h"
#include <eco/persist/Sqlite.h>
#include <eco/persist/SqliteVersion.h>
#pragma message(ECO_LIB_NAME(eco_sqlite))
////////////////////////////////////////////////////////////////////////////////
#include <sqlite3.h>
#include <eco/std/mutex.h>
#include <eco/rx/RxImpl.h>
#include <eco/thread/Lock.h>
#include <boost/filesystem/operations.hpp>


////////////////////////////////////////////////////////////////////////////////
eco::Database* create() { return new eco::Sqlite(); }
ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
thread_local uint32_t t_trans_level = 0;
inline bool has_transaction()
{
	return t_trans_level > 0;
}


////////////////////////////////////////////////////////////////////////////////
class SqliteConfig : public eco::DatabaseConfig
{
public:
	virtual void get_field_type_sql(
		OUT std::string& field_sql,
		IN  const uint32_t field_type,
		IN  const uint32_t field_size) override
	{
		switch (field_type)
		{
		case type_date_time:
			PropertyMapping::get_field_type_sql(
				field_sql, field_char_array, 19);
			break;
		}
	}

	// get config info.
	virtual const char* get_database() override
	{
		return m_db_name.c_str();
	}

	// get index sql.
	virtual bool get_index_sql(
		OUT std::string& sql,
		IN  const PropertyMapping& pmap) const override
	{
		return false;		// sqlite don't support index.
	}

	inline void set_charset(IN const persist::Charset v)
	{
		switch (v)
		{
		case persist::charset_gbk:
			m_charset = "gbk";
			return;
		case persist::charset_gb2312:
			m_charset = "gb2312";
			return;
		case persist::charset_utf8:
			m_charset = "utf8";
			return;
		case persist::charset_utf16:
			m_charset = "utf16";
			return;
		case persist::charset_utf32:
			m_charset = "utf32";
			return;
		}
		m_charset = "gbk";	// default is gbk.
	}

public:
	// connection info.
	std::string m_db_name;
	std::string m_charset;
};


////////////////////////////////////////////////////////////////////////////////
class Sqlite::Impl
{
	ECO_IMPL_INIT(Sqlite);
public:
	// mysql instance.
	sqlite3* m_sqlite;
	std_mutex m_sqlite_mutex;
	// config
	SqliteConfig m_config;
	
public:
	inline Impl() : m_sqlite(nullptr)
	{}

	inline ~Impl()
	{
		close();
	}

	inline void open()
	{
		boost::system::error_code ec;
		boost::filesystem::path path(m_config.m_db_name);
		path = path.parent_path();
		if (!boost::filesystem::exists(path, ec) &&
			!boost::filesystem::create_directory(path, ec))
		{
			eco::String err;
			err << "create directory fail" << m_config.m_db_name;
			throw std::logic_error(err.c_str());
		}

		// create sqlite instance.
		sqlite3* sqlite = nullptr;
		if (sqlite3_open(m_config.m_db_name.c_str(), &sqlite))
		{
			throw_error();
		}
		m_sqlite = sqlite;
	}

	// connect to server once.
	inline void open_once()
	{
		if (m_sqlite == nullptr)
		{
			open();
		}
	}

	// close mysql.
	inline void close()
	{
		if (nullptr != m_sqlite)
		{
			sqlite3_close(m_sqlite);
			m_sqlite = nullptr;
		}
	}

	// throw error.
	inline void throw_error(IN const char* sql = nullptr)
	{
		throw_error(sql, sqlite3_errcode(m_sqlite),
			sqlite3_errmsg(m_sqlite));
	}

	// throw error.
	inline void throw_error(
		IN const char* sql,
		IN int err_no,
		IN const char* err_msg)
	{
		std::string msg("sqlite fail: ");
		msg += err_msg;
		if (sql != nullptr)
		{
			msg += " sql:";
			msg += sql;
		}
		throw std::logic_error(msg);
	}

	// query sql.
	inline void QuerySql(IN const char* sql)
	{
		const char* tail = nullptr;
		sqlite3_stmt* stmt = nullptr;
		if (0 != sqlite3_prepare(m_sqlite, sql, -1, &stmt, &tail))
		{
			throw_error(sql);
		}// end if.
	}
};
ECO_SHARED_IMPL(Sqlite);
////////////////////////////////////////////////////////////////////////////////
void Sqlite::open(IN const persist::Address& addr)
{
	open(addr.database(), addr.charset());
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::open(
	IN const char* db_name,
	IN const persist::Charset charset)
{
	eco::ScopeLock lock(impl().m_sqlite_mutex);
	impl().m_config.m_db_name = db_name;
	impl().m_config.set_charset(charset);
	impl().open();
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::close()
{
	eco::ScopeLock lock(impl().m_sqlite_mutex);
	impl().close();
}


////////////////////////////////////////////////////////////////////////////////
bool Sqlite::is_open()
{
	eco::ScopeLock lock(impl().m_sqlite_mutex, !has_transaction());
	return (impl().m_sqlite != nullptr);
}


////////////////////////////////////////////////////////////////////////////////
bool Sqlite::select(OUT Record& obj, IN  const char* sql)
{
	Recordset rset;
	select(rset, sql);
	if (rset.size() > 0)
	{
		obj = std::move(rset[0]);
		return true;
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::select(OUT Recordset& rd_set, IN  const char* sql)
{
	eco::ScopeLock lock(impl().m_sqlite_mutex, !has_transaction());
	impl().open_once();

	// query sql.
	const char* tail = nullptr;
	sqlite3_stmt* stmt = nullptr;
	if (0 != sqlite3_prepare(impl().m_sqlite, sql, -1, &stmt, &tail))
	{
		impl().throw_error(sql);
	}// end if.

	// query success, get query result.
	int col_size = sqlite3_column_count(stmt);
	for (int row=sqlite3_step(stmt); row==SQLITE_ROW; row=sqlite3_step(stmt))
	{
		Record& item = rd_set.add_item();
		item.reserve(col_size);
		for (int col=0; col<col_size; ++col)
		{
			const char* v = (sqlite3_column_type(stmt, col) == SQLITE_NULL) 
				? "" : (const char*)sqlite3_column_text(stmt, col);
			item.add(v);
		}
	}

	// close stmt
	sqlite3_finalize(stmt);
}


////////////////////////////////////////////////////////////////////////////////
uint64_t Sqlite::execute_sql(IN const char* sql)
{
	eco::ScopeLock lock(impl().m_sqlite_mutex, !has_transaction());
	impl().open_once();

	// execute sql fail.
	char* err_msg = nullptr;
	int err_no = sqlite3_exec(impl().m_sqlite, sql, 0, 0, &err_msg);
	if (err_no != SQLITE_OK)
	{
		impl().throw_error(sql, err_no, err_msg);
	}

	// effect rows.
	int rows = sqlite3_changes(impl().m_sqlite);
	return rows;
}


////////////////////////////////////////////////////////////////////////////////
bool Sqlite::ping()
{
	return true;
}


////////////////////////////////////////////////////////////////////////////////
uint64_t Sqlite::get_system_time()
{
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::get_tables(OUT Recordset& tables, IN  const char* db_name)
{
	std::string cond_sql("select name from sqlite_master"
		" where type='table' order by name;");
	return select(tables, cond_sql.c_str());
}


////////////////////////////////////////////////////////////////////////////////
bool Sqlite::has_table(IN const char* table_name, IN const char* db_name)
{
	if (db_name == 0) db_name = config().get_database();
	std::string cond_sql("where type='table' and name='");
	cond_sql += table_name;
	cond_sql += "'";
	return has_record("sqlite_master", cond_sql.c_str()) > 0;
}


////////////////////////////////////////////////////////////////////////////////
bool Sqlite::has_field(const char* table, const char* field, const char* db)
{
	Recordset rec_set;
	std::string sql("PRAGMA TABLE_INFO(");
	sql += table;
	sql += ")";
	select(rec_set, sql.c_str());
	for (size_t i = rec_set.size() - 1; i != size_t(-1); --i)
	{
		auto& rec = rec_set.at(i);
		if (eco::equal(rec.at(1), field))
		{
			return true;
		}
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::set_field(
	IN const char* table,
	IN const PropertyMapping& prop,
	IN const char* db_name)
{
	char sql[128] = { 0 };
	if (db_name == 0) db_name = config().get_database();
	if (!has_field(table, prop.field(), db_name))
	{
		sprintf(sql, "alter table %s.%s add %s %s", db_name, table,
			prop.field(), prop.get_field_type_sql(&config()).c_str());
		execute_sql(sql);
	}
	else
	{
		eco_cpyc(sql, "sqlite don't support modify table column(field): ");
		eco_catc(sql, prop.field());
		throw std::logic_error(sql);
	}
}


////////////////////////////////////////////////////////////////////////////////
bool Sqlite::has_index(const char* table, const char* field, const char* db)
{
	return false;
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::set_index(
	IN const char* table,
	IN const PropertyMapping& prop,
	IN const char* db_name)
{
	
}



////////////////////////////////////////////////////////////////////////////////
DatabaseConfig& Sqlite::config()
{
	return impl().m_config;
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::begin()
{
	impl().m_sqlite_mutex.lock();
	execute_sql("BEGIN;");
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::commit()
{
	execute_sql("COMMIT;");
	impl().m_sqlite_mutex.unlock();
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::rollback()
{
	execute_sql("ROLLBACK;");
	impl().m_sqlite_mutex.unlock();
}


////////////////////////////////////////////////////////////////////////////////
uint32_t& Sqlite::transaction_level()
{
	return t_trans_level;
}
////////////////////////////////////////////////////////////////////////////////
}// ns