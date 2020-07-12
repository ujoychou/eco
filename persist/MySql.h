#ifndef ECO_MYSQL_H
#define ECO_MYSQL_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/persist/Database.h>

#ifndef ECO_AUTO_LINK_NO
#	if !defined(ECO_EXPORT)
#		pragma comment(lib, "eco_mysql.lib")
#	endif
#endif
extern "C" ECO_API eco::Database* create();


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
class ECO_API MySql : public eco::Database
{
	ECO_SHARED_API(MySql);
////////////////////////////////////////////////////////////////////////////////
public:
	// get mysql server config.
	virtual DatabaseConfig& config() override;

	// create database
	virtual void create_database(
		IN const char* db_name) override;

	// connect to database address.
	virtual void open(
		IN const persist::Address& addr) override;

	// connect to server and login database.
	virtual void open(
		IN const char* server_ip,
		IN const uint32_t port,
		IN const char* db_name,
		IN const char* user_id,
		IN const char* password,
		IN const persist::Charset char_set = persist::charset_gbk) override;

	// disconnect to server
	virtual void close() override;

	// get the state of database connection.
	virtual bool is_open() override;

	// read the object from database.
	virtual bool select(
		OUT Record& obj,
		IN  const char* sql) override;

	// read data sheet from database.
	virtual void select(
		OUT Recordset& obj_set,
		IN  const char* sql) override;

	// execute sql, return affected rows.
	virtual uint64_t execute_sql(
		IN  const char* sql) override;

	// ping mysql server: if success return true, else false.
	virtual bool ping() override;

	// get mysql server system time.
	// time format: 20160310202020
	virtual uint64_t get_system_time() override;

	// is exist table in this datasource.
	virtual bool has_table(
		IN const char* table_name,
		IN  const char* db_name = nullptr) override;

	// get exist tables in this datasource.
	virtual void get_tables(
		OUT Recordset& tables,
		IN  const char* db_name = nullptr) override;

	// is exist field in this table.
	virtual bool has_field(
		IN const char* table_name,
		IN const char* field_name,
		IN const char* db_name = nullptr) override;

	// set field in table, add field if the field is not exist else modify it.
	virtual void set_field(
		IN const char* table,
		IN const PropertyMapping& prop,
		IN const char* db_name = nullptr) override;

	// is exist index in this table.
	virtual bool has_index(
		IN const char* table_name,
		IN const char* index_name,
		IN const char* db_name = nullptr) override;

	// set index in table, add index if the field is not exist else modify it.
	virtual void set_index(
		IN const char* table,
		IN const PropertyMapping& prop,
		IN const char* db_name = nullptr) override;

public:
	// begin a transaction.
	virtual void begin() override;

	// commit a transaction.
	virtual void commit() override;

	// rollback a transaction.
	virtual void rollback() override;

	// trans level management.
	virtual uint32_t& transaction_level() override;
};


////////////////////////////////////////////////////////////////////////////////
}// ns::afw
#endif