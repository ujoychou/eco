#ifndef ECO_SQLITE_H
#define ECO_SQLITE_H
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
#		pragma comment(lib, "eco_sqlite.lib")
#	endif
#endif

extern "C" ECO_API eco::Database* create();


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
class ECO_API Sqlite : public eco::Database
{
	ECO_SHARED_API(Sqlite);
////////////////////////////////////////////////////////////////////////////////
public:
	// connect to database address.
	virtual void open(
		IN const persist::Address& addr) override;

	// connect to server and login database.
	virtual void open(
		IN const char* db_name,
		IN const persist::CharSet char_set = persist::char_set_gbk) override;

	// disconnect to server
	virtual void close() override;

	// get the state of database connection.
	virtual bool is_open() override;

	// read the object from database.
	virtual void select(
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
		IN const char* table_name) override;

	// get exist tables in this datasource.
	virtual void get_tables(
		OUT Recordset& tables,
		IN  const char* db_name = "") override;

	// get mysql server config.
	virtual DatabaseConfig& config() override;

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