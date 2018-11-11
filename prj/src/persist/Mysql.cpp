#include "PrecHeader.h"
#include <eco/persist/MySql.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Type.h>
#include <eco/thread/Atomic.h>
#include <eco/thread/Mutex.h>
#include <vector>
#include <mysql/mysql.h>


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
};


////////////////////////////////////////////////////////////////////////////////
class MySql::Impl
{
	ECO_IMPL_INIT(MySql);
public:
	// connection info.
	int m_port;
	std::string m_server_ip;
	std::string m_db_name;
	std::string m_user_id;
	std::string m_password;
	std::string m_charset;
	// mysql instance.
	MYSQL* m_mysql;
	eco::Mutex m_mysql_mutex;

	// mysql config.
	MySqlConfig m_config;

public:
	inline Impl() : m_mysql(nullptr), m_port(0)
	{}

	inline ~Impl()
	{
		close();
	}

	void set_charset(IN const persist::Charset v)
	{
		switch (v)
		{
		case persist::charset_gbk :
			m_charset = "gbk";
			return ;
		case persist::charset_gb2312 :
			m_charset = "gb2312";
			return ;
		case persist::charset_utf8 :
			m_charset = "utf8";
			return ;
		case persist::charset_utf16 :
			m_charset = "utf16";
			return ;
		case persist::charset_utf32 :
			m_charset = "utf32";
			return ;
		}
		m_charset = "gbk";	// default is gbk.
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
			m_server_ip.c_str(), 
			m_user_id.c_str(),
			m_password.c_str(),
			nullptr,
			m_port, nullptr, 0))
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
		if (mysql_set_character_set(m_mysql, m_charset.c_str()) != 0)
		{
			close_throw_open_error("charset");
		}

		// create database.
		create_database(m_db_name.c_str());

		// connect to mysql server success.
		// connect to dedicated server.
		if (0 != mysql_select_db(m_mysql, m_db_name.c_str()))
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
	inline void throw_error(
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
		throw std::logic_error(msg.c_str());
	}
	
	// reconnect mysql.
	inline void reconnect()
	{
		// reconnect 3 times.
		for (int i=0; i<3; ++i)
		{
			try
			{
				close();
				open();
				break;
			}
			catch (std::exception& e)
			{
				e.what();
			}
		}// end for.
	}

	// query sql.
	inline void query_sql(IN const char* sql)
	{
		if (0 != mysql_query(m_mysql, sql))
		{
			uint32_t eno = mysql_errno(m_mysql);
			// 2013: lost connetion during query.
			// 2006
			if (eno == 2013 || eno == 2006)
			{
				const char* err = mysql_error(m_mysql);
				close();
				throw_error("mysql query", sql, err, eno);
			}
			if (eno != 0)
			{
				throw_query_error(sql, eno);
			}

			// 1064: sql syntax error.
			// 1146: table doesn't exist.
			reconnect();
			if (0 != mysql_query(m_mysql, sql))
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

	inline void create_database(IN const char* db_name)
	{
		std::string sql("create database if not exists ");
		sql += db_name;
		sql += " character set = utf8";
		execute_sql(sql.c_str());
	}
};
ECO_SHARED_IMPL(MySql);
////////////////////////////////////////////////////////////////////////////////
void MySql::create_database(IN const char* db_name)
{
	std::string sql("create database if not exists ");
	sql += db_name;
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
	impl().m_port = port;
	impl().m_db_name = db_name;
	impl().m_user_id = user_id;
	impl().m_password = password;
	impl().m_server_ip = server_ip;
	impl().set_charset(charset);
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
void MySql::select(OUT Record& obj, IN  const char* sql)
{
	Recordset data_sheet;
	select(data_sheet, sql);
	if (data_sheet.size() > 0)
	{
		obj = data_sheet[0];
	}
}


////////////////////////////////////////////////////////////////////////////////
void MySql::select(
	OUT Recordset& obj_sheet,
	IN  const char* sql)
{
	eco::Mutex::ScopeLock lock(impl().m_mysql_mutex, !has_transaction());
	impl().open_once();

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
	impl().open_once();

	// query sql fail, reconnect it.
	impl().query_sql(sql);
	return mysql_affected_rows(impl().m_mysql);
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
bool MySql::has_table(IN const char* table_name)
{
	std::string cond_sql("where table_name='"); 
	cond_sql += table_name;
	cond_sql += "' and TABLE_SCHEMA='";
	cond_sql += impl().m_db_name;
	cond_sql += "'";
	return has_record("information_schema.TABLES", cond_sql.c_str()) > 0;
}


////////////////////////////////////////////////////////////////////////////////
void MySql::get_tables(OUT Recordset& tables, IN  const char* db)
{
	char cond_sql[128] = { 0 };
	const char* name = eco::empty(db) ? impl().m_db_name.c_str() : db;
	sprintf(cond_sql, "select TABLE_NAME from information_schema.TABLES"
		" where TABLE_SCHEMA='%s'", name);
	return select(tables, cond_sql);
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
	execute_sql("ROLLBACK;");
	impl().m_mysql_mutex.unlock();
}


////////////////////////////////////////////////////////////////////////////////
uint32_t& MySql::transaction_level()
{
	return t_trans_level;
}


////////////////////////////////////////////////////////////////////////////////
}// ns