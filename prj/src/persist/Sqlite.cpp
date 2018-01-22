#include "PrecHeader.h"
#include <eco/persist/Sqlite.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/thread/Atomic.h>
#include <eco/thread/Mutex.h>
#include <vector>
#include <sqlite/sqlite3.h>


////////////////////////////////////////////////////////////////////////////////
eco::Database* create()
{
	return new eco::Sqlite();
}


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
__declspec(thread) uint32_t	t_trans_level = 0;
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
				field_sql, type_char_array, 19);
			break;
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
class Sqlite::Impl
{
	ECO_IMPL_INIT(Sqlite);
public:
	// connection info.
	std::string m_db_name;
	std::string m_char_set;
	// mysql instance.
	sqlite3* m_sqlite;
	eco::Mutex m_sqlite_mutex;
	// config
	SqliteConfig m_config;
	
public:
	inline Impl() : m_sqlite(nullptr)
	{}

	inline ~Impl()
	{
		close();
	}
	
	inline void set_charset(IN const persist::CharSet v)
	{
		switch (v)
		{
		case persist::char_set_gbk :
			m_char_set = "gbk";
			return ;
		case persist::char_set_gb2312 :
			m_char_set = "gb2312";
			return ;
		case persist::char_set_utf8 :
			m_char_set = "utf8";
			return ;
		case persist::char_set_utf16 :
			m_char_set = "utf16";
			return ;
		case persist::char_set_utf32 :
			m_char_set = "utf32";
			return ;
		}
		m_char_set = "gbk";	// default is gbk.
	}

	inline void open()
	{
		// create sqlite instance.
		if (sqlite3_open(m_db_name.c_str(), &m_sqlite))
		{
			throw_error();
		}
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
	open(addr.get_database(), addr.get_char_set());
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::open(
	IN const char* db_name,
	IN const persist::CharSet char_set)
{
	eco::Mutex::ScopeLock lock(impl().m_sqlite_mutex);
	impl().m_db_name = db_name;
	impl().set_charset(char_set);
	impl().open();
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::close()
{
	eco::Mutex::ScopeLock lock(impl().m_sqlite_mutex);
	impl().close();
}


////////////////////////////////////////////////////////////////////////////////
bool Sqlite::is_open()
{
	eco::Mutex::ScopeLock lock(impl().m_sqlite_mutex, !has_transaction());
	return (impl().m_sqlite != nullptr);
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::select(OUT Record& obj, IN  const char* sql)
{
	Recordset rset;
	select(rset, sql);
	if (rset.size() > 0)
	{
		obj = rset[0];
	}
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::select(
	OUT Recordset& rd_set,
	IN  const char* sql)
{
	eco::Mutex::ScopeLock lock(impl().m_sqlite_mutex, !has_transaction());
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
	eco::Mutex::ScopeLock lock(impl().m_sqlite_mutex, !has_transaction());
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
bool Sqlite::has_table(IN const char* table_name)
{
	std::string cond_sql("where type='table' and name='");
	cond_sql += table_name;
	cond_sql += "'";
	return has_record("sqlite_master", cond_sql.c_str()) > 0;
}


////////////////////////////////////////////////////////////////////////////////
void Sqlite::get_tables(OUT Recordset& tables, IN  const char* db_name)
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