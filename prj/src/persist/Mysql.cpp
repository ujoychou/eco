#include "PrecHeader.h"
#include <eco/persist/MySql.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Type.h>
#include <eco/thread/Atomic.h>
#include <eco/thread/Mutex.h>
#include <vector>
#include <mysql/mysql.h>

// import eco/log.
#undef ECO_API
#define ECO_API __declspec(dllimport)
#include <eco/log/Log.h>
#pragma comment(lib, "eco.lib")
#undef ECO_API
#define ECO_API __declspec(dllexport)


////////////////////////////////////////////////////////////////////////////////
eco::Database* create()
{
	return new eco::MySql();
}


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
__declspec(thread) uint32_t	t_trans_level = 0;
inline bool has_transaction()
{
	return t_trans_level > 0;
}


////////////////////////////////////////////////////////////////////////////////
class MySqlConfig : public eco::DatabaseConfig
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
			field_sql = "DATETIME";
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
		return pmap.get_index_sql(sql);
	}

public:
	inline MySqlConfig() : m_port(0)
	{}

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
		m_charset = "utf8";	// default is utf8.
	}

public:
	// connection info.
	int m_port;
	std::string m_server_ip;
	std::string m_db_name;
	std::string m_user_id;
	std::string m_password;
	std::string m_charset;
};


////////////////////////////////////////////////////////////////////////////////
class MySql::Impl
{
	ECO_IMPL_INIT(MySql);
public:
	// mysql instance.
	MYSQL* m_mysql;
	eco::Mutex m_mysql_mutex;

	// mysql config.
	MySqlConfig m_config;

public:
	inline Impl() : m_mysql(nullptr)
	{}

	inline ~Impl()
	{
		close();
	}

	void open()
	{
		close();

		// init mysql instance.
		m_mysql = mysql_init(nullptr);
		if (nullptr == m_mysql)
		{
			throw_error("mysql init", nullptr, nullptr, 0);
		}

		// connect to mysql server fail.
		if (nullptr == mysql_real_connect(m_mysql,
			m_config.m_server_ip.c_str(),
			m_config.m_user_id.c_str(),
			m_config.m_password.c_str(),
			nullptr, m_config.m_port, nullptr, 0))
		{
			close_throw_open_error("connect");
		}

		// set autocommit=0 transaction.
		/*if (mysql_autocommit(m_mysql, false) != 0)
		{
			close_throw_open_error("auto commit");
		}*/

		// set mysql client options.
		// set mysql client character set.
		if (mysql_set_character_set(m_mysql, m_config.m_charset.c_str()) != 0)
		{
			close_throw_open_error("charset");
		}

		// create database.
		create_database(m_config.m_db_name.c_str());

		// connect to mysql server success.
		// connect to dedicated server.
		if (0 != mysql_select_db(m_mysql, m_config.m_db_name.c_str()))
		{
			close_throw_open_error("select db");
		}
	}

	// connect to server once.
	inline void open_once()
	{
		if (m_mysql == nullptr)
		{
			open();
		}
	}

	// close mysql.
	inline void close()
	{
		if (nullptr != m_mysql)
		{
			mysql_close(m_mysql);
			m_mysql = nullptr;
		}
	}

	// ping mysql server: if success return true, else false.
	inline bool ping()
	{
		return (mysql_ping(m_mysql) == 0) ? true : false;
	}

public:
	// throw open error.
	inline void close_throw_open_error(IN const char* t)
	{
		int eid = mysql_errno(m_mysql);
		std::string emsg(mysql_error(m_mysql));
		std::string title("mysql open ");
		title += t;

		close();
		throw_error(title.c_str(), nullptr, emsg.c_str(), eid);
	}

	// throw result error.
	inline void throw_result_error(IN const char* sql)
	{
		throw_error("mysql result", sql,
			mysql_error(m_mysql), mysql_errno(m_mysql));
	}

	// throw query error.
	inline void throw_query_error(IN const char* sql, IN int err_no)
	{
		if (err_no == 0)
		{
			err_no = mysql_errno(m_mysql);
		}
		throw_error("mysql query", sql, mysql_error(m_mysql), err_no);
	}

	// throw detail error.
	inline std::string get_error_msg(
		IN const char* title,
		IN const char* sql,
		IN const char* err,
		IN int eno)
	{
		std::string msg(title);
		msg.reserve(256);
		msg += " fail: ";
		if (err != nullptr)
		{
			msg += err;
		}
		if (eno != 0)
		{
			msg += " #e";
			msg += eco::cast<std::string>(eno);
		}
		if (sql != nullptr)
		{
			msg += " sql: ";
			msg += sql;
		}
		return msg;
	}
	inline void throw_error(
		IN const char* title,
		IN const char* sql,
		IN const char* err,
		IN int eno)
	{
		throw std::logic_error(get_error_msg(title, sql, err, eno).c_str());
	}
	
	// reconnect mysql.
	inline void reconnect()
	{
		// reconnect 3 times.
		for (int i = 0; i < 3; ++i)
		{
			try
			{
				close();
				open();
				break;
			}
			catch (std::exception& e)
			{
				close();
				ECO_ERROR << e.what();
			}
		}// end for.
	}

	// query sql.
	inline void query_sql(IN const char* sql)
	{
		open_once();
		if (0 != mysql_query(m_mysql, sql))
		{
			/* if mysql fail, reconnect 3 times.
			e2013: lost connetion during query.
			e2006: mysql server has gone away.
			e1064: sql syntax error.
			e1146: table doesn't exist.
			e2008: mysql client ran out of memory.
			*/

			// auto reconnect when mysql connect has gone.
			uint32_t eno = mysql_errno(m_mysql);
			if (eno == 2013 || eno == 2006)
			{
				const char* err = mysql_error(m_mysql);
				ECO_ERROR << get_error_msg("mysql query", sql, err, eno);
			}
			else if (eno != 0)
			{
				throw_query_error(sql, eno);
			}

			// restart execute sql.
			reconnect();
			if (m_mysql == nullptr)
			{
				throw_error("mysql reconnect", sql, nullptr, 0);
			}
			if (mysql_query(m_mysql, sql) != 0)
			{
				throw_query_error(sql, 0);
			}
		}// end if.
	}

	inline uint64_t execute_sql(IN const char* sql)
	{
		// query sql fail, reconnect it.
		query_sql(sql);
		return mysql_affected_rows(m_mysql);
	}

	inline void create_database(IN const char* db)
	{
		std::string sql("create database if not exists ");
		sql += db;
		sql += " character set = utf8";
		execute_sql(sql.c_str());
	}
};
ECO_SHARED_IMPL(MySql);
////////////////////////////////////////////////////////////////////////////////
void MySql::create_database(IN const char* db)
{
	std::string sql("create database if not exists ");
	sql += db;
	sql += " character set = utf8";
	execute_sql(sql.c_str());
}


////////////////////////////////////////////////////////////////////////////////
void MySql::open(IN const persist::Address& addr) 
{
	open(addr.get_host(), addr.get_port(), addr.get_database(),
		addr.get_user(), addr.get_password(), addr.get_charset());
}


////////////////////////////////////////////////////////////////////////////////
void MySql::open(
	IN const char* server_ip,
	IN const uint32_t port,
	IN const char* db_name,
	IN const char* user_id,
	IN const char* password,
	IN const persist::Charset charset)
{
	eco::Mutex::ScopeLock lock(impl().m_mysql_mutex);
	impl().m_config.m_port = port;
	impl().m_config.m_db_name = db_name;
	impl().m_config.m_user_id = user_id;
	impl().m_config.m_password = password;
	impl().m_config.m_server_ip = server_ip;
	impl().m_config.set_charset(charset);
	impl().open();
}


////////////////////////////////////////////////////////////////////////////////
void MySql::close()
{
	eco::Mutex::ScopeLock lock(impl().m_mysql_mutex);
	impl().close();
}


////////////////////////////////////////////////////////////////////////////////
bool MySql::is_open()
{
	eco::Mutex::ScopeLock lock(impl().m_mysql_mutex, !has_transaction());
	return (impl().m_mysql != nullptr);
}


////////////////////////////////////////////////////////////////////////////////
bool MySql::select(OUT Record& obj, IN  const char* sql)
{
	Recordset data_sheet;
	select(data_sheet, sql);
	if (data_sheet.size() > 0)
	{
		obj = data_sheet[0];
		return true;
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////
void MySql::select(OUT Recordset& obj_sheet, IN const char* sql)
{
	eco::Mutex::ScopeLock lock(impl().m_mysql_mutex, !has_transaction());

	// query sql fail, reconnect it.
	impl().query_sql(sql);

	// query success, get query result.
	MYSQL_RES* res = mysql_store_result(impl().m_mysql);
	if (nullptr == res)
	{
		impl().throw_result_error(sql);
	}

	// get data row size.(object size.)
	uint64_t row_size = mysql_num_rows(res);
	if (row_size == 0)
	{
		return ;
	}

	// iterate the object row.
	MYSQL_ROW row = mysql_fetch_row(res);
	uint64_t field_size = mysql_num_fields(res);
	obj_sheet.reserve((size_t)(obj_sheet.size() + row_size));
	for (uint32_t r=0; row!=nullptr && r<row_size; ++r)
	{
		Record& item = obj_sheet.add_item();
		item.reserve((size_t)field_size);
		for (uint32_t f=0; f<field_size; ++f)
		{
			const char* v = row[f];
			item.add(v != nullptr ? v : "");
		}
		row = mysql_fetch_row(res);
	}
	// free query result.
	mysql_free_result(res);
}


////////////////////////////////////////////////////////////////////////////////
uint64_t MySql::execute_sql(IN const char* sql)
{
	eco::Mutex::ScopeLock lock(impl().m_mysql_mutex, !has_transaction());

	// query sql fail, reconnect it.
	return impl().execute_sql(sql);
}


////////////////////////////////////////////////////////////////////////////////
bool MySql::ping()
{
	eco::Mutex::ScopeLock lock(impl().m_mysql_mutex, !has_transaction());
	impl().open_once();
	return impl().ping();
}


////////////////////////////////////////////////////////////////////////////////
uint64_t MySql::get_system_time()
{
	const char* sql = "select now()";
	Record rd;
	select(rd, sql);

	// system time format: 2016-03-10 17:41:40.
	if (rd.size() != 0)		// parse system time.
	{
		std::string temp = rd[0];
		temp.erase(16, 1);
		temp.erase(13, 1);
		temp.erase(10, 1);
		temp.erase(7, 1);
		temp.erase(4, 1);
		return _atoi64(temp.c_str());
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
bool MySql::has_table(IN const char* table, IN const char* db)
{
	char cond_sql[128] = { 0 };
	if (db == nullptr) db = impl().m_config.m_db_name.c_str();
	sprintf(cond_sql, "where TABLE_SCHEMA='%s' and TABLE_NAME='%s'", db, table);
	return has_record("information_schema.TABLES", cond_sql) > 0;
}


////////////////////////////////////////////////////////////////////////////////
void MySql::get_tables(OUT Recordset& tables, IN  const char* db)
{
	char cond_sql[128] = { 0 };
	if (db == nullptr) db = impl().m_config.m_db_name.c_str();
	sprintf(cond_sql, "select TABLE_NAME from information_schema.TABLES"
		" where TABLE_SCHEMA='%s'", db);
	return select(tables, cond_sql);
}


////////////////////////////////////////////////////////////////////////////////
bool MySql::has_field(const char* table, const char* field, const char* db)
{
	char cond_sql[256] = { 0 };
	if (db == nullptr) db = impl().m_config.m_db_name.c_str();
	sprintf(cond_sql, "where TABLE_SCHEMA='%s' and TABLE_NAME='%s' and "
		"COLUMN_NAME='%s'", db, table, field);
	return has_record("information_schema.COLUMNS", cond_sql) > 0;
}


////////////////////////////////////////////////////////////////////////////////
void MySql::set_field(
	IN const char* table,
	IN const PropertyMapping& prop,
	IN const char* db)
{
	char sql[128] = { 0 };
	if (db == 0) db = config().get_database();
	if (!has_field(table, prop.get_field(), db))
	{
		sprintf(sql, "alter table %s.%s add %s %s", db, table,
			prop.get_field(), prop.get_field_type_sql(&config()).c_str());
	}
	else
	{
		sprintf(sql, "alter table %s.%s modify %s %s", db, table,
			prop.get_field(), prop.get_field_type_sql(&config()).c_str());
	}
	execute_sql(sql);
}


////////////////////////////////////////////////////////////////////////////////
bool MySql::has_index(const char* table, const char* index, const char* db)
{
	char cond_sql[256] = { 0 };
	if (db == nullptr) db = impl().m_config.m_db_name.c_str();
	sprintf(cond_sql, "where TABLE_SCHEMA='%s' and TABLE_NAME='%s' and "
		"INDEX_NAME='%s'", db, table, index);
	return has_record("information_schema.STATISTICS", cond_sql) > 0;
}


////////////////////////////////////////////////////////////////////////////////
void MySql::set_index(
	IN const char* table,
	IN const PropertyMapping& prop,
	IN const char* db)
{
	char sql[128] = { 0 };
	if (db == 0) db = config().get_database();
	if (has_index(table, prop.get_index_name().c_str(), db))
	{
		sprintf(sql, "drop index %s on %s.%s",
			prop.get_index_name().c_str(), db, table);
		execute_sql(sql);
	}

	std::string index_sql;
	prop.get_index_sql(index_sql);
	sprintf(sql, "alter table %s.%s add %s", db, table, index_sql.c_str());
	execute_sql(sql);
}


////////////////////////////////////////////////////////////////////////////////
DatabaseConfig& MySql::config()
{
	return impl().m_config;
}


////////////////////////////////////////////////////////////////////////////////
void MySql::begin()
{
	impl().m_mysql_mutex.lock();
	execute_sql("BEGIN;");
}


////////////////////////////////////////////////////////////////////////////////
void MySql::commit()
{
	execute_sql("COMMIT;");
	impl().m_mysql_mutex.unlock();
}


////////////////////////////////////////////////////////////////////////////////
void MySql::rollback()
{
	try { execute_sql("ROLLBACK;"); } catch (...) {}
	impl().m_mysql_mutex.unlock();
}


////////////////////////////////////////////////////////////////////////////////
uint32_t& MySql::transaction_level()
{
	return t_trans_level;
}


////////////////////////////////////////////////////////////////////////////////
}// ns