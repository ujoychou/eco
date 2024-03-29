#ifndef ECO_DATABASE_H
#define ECO_DATABASE_H
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
#include <eco/Error.h>
#include <eco/persist/Recordset.h>
#include <eco/persist/JoinMapping.h>
#include <eco/persist/DatabaseConfig.h>
#include <eco/persist/Address.h>
#ifndef ECO_NO_PROTOBUF
#include <eco/detail/proto/Proto.h>
#endif


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class ECO_API Database
{
	ECO_OBJECT(Database);
public:
	inline Database() {}

	// get mysql server config.
	virtual DatabaseConfig& config() = 0;

	// create database
	virtual void create_database(
		IN const char* db_name) = 0;

	// connect to database address.
	virtual void open(
		IN const persist::Address& addr) {};

	// connect to server and login database.
	virtual void open(
		IN const char* server_ip,
		IN const uint32_t port,
		IN const char* db_name,
		IN const char* user_id,
		IN const char* password,
		IN const persist::Charset char_set = persist::charset_gbk) {};

	// connect to sqlite database.
	virtual void open(
		IN const char* db_name,
		IN const persist::Charset char_set = persist::charset_gbk) {};

	// disconnect to server
	virtual void close() = 0;

	// get the state of database connection.
	virtual bool is_open() = 0;

	// read the object from database.
	virtual bool select(
		OUT Record& obj,
		IN  const char* sql) = 0;

	// read data sheet from database.
	virtual void select(
		OUT Recordset& obj_set,
		IN  const char* sql) = 0;

	// execute sql, return affected rows.
	virtual uint64_t execute_sql(
		IN  const char* sql) = 0;

	// ping mysql server: if success return true, else false.
	virtual bool ping() = 0;

	// get mysql server system time.
	virtual uint64_t get_system_time() = 0;

	// is exist table in this datasource.
	virtual bool has_table(
		IN const char* table_name,
		IN const char* db_name = nullptr) = 0;

	// get exist tables in this datasource.
	virtual void get_tables(
		OUT Recordset& tables,
		IN  const char* db_name = nullptr) = 0;

	// is exist field in this table.
	virtual bool has_field(
		IN const char* table_name,
		IN const char* field_name,
		IN const char* db_name = nullptr) = 0;

	// set field in table, add field if the field is not exist else modify it.
	virtual void set_field(
		IN const char* table,
		IN const PropertyMapping& prop,
		IN const char* db_name = nullptr) = 0;

	// is exist index in this table.
	virtual bool has_index(
		IN const char* table_name,
		IN const char* index_name,
		IN const char* db_name = nullptr) = 0;

	// set index in table, add index if the field is not exist else modify it.
	virtual void set_index(
		IN const char* table,
		IN const PropertyMapping& prop,
		IN const char* db_name = nullptr) = 0;

////////////////////////////////////////////////////////////////////////////////
public:
	////////////////////////////////////////////////////////////////////////////////
	/* auto transaction management.
	1.multi thread will execute the transaction sync.
	2.transaction level is thread safe, it is a thread local data.
	3.this thread manage a level transaction.
	*/
	class Transaction
	{
	public:
		inline Transaction(eco::Database& db) : m_db(db), m_commited(false)
		{
			++m_db.transaction_level();
			if (m_db.transaction_level() == 1)
			{
				m_db.begin();
			}
		}

		// auto rollback when hasn't call commit explicit.
		inline ~Transaction()
		{
			if (!m_commited)
			{
				m_db.rollback();
				m_db.transaction_level() = 0;
			}
		}

		// commit when database operation finished.
		inline void commit()
		{
			if (m_db.transaction_level() == 0)
			{
				throw std::logic_error("sql [commit] fail with no transaction.");
			}
			if (m_db.transaction_level() == 1)
			{
				m_db.commit();
			}
			--m_db.transaction_level();
			m_commited = true;
		}

		inline eco::Database& db()
		{
			return m_db;
		}

	private:
		eco::Database& m_db;
		uint32_t m_commited;
	};


	// begin a transaction.
	virtual void begin() = 0;

	// commit a transaction.
	virtual void commit() = 0;

	// rollback a transaction.
	virtual void rollback() = 0;

	// transaction level, thread local data.
	virtual uint32_t& transaction_level() = 0;

////////////////////////////////////////////////////////////////////////////////
public:
	// get database name.
	const char* get_database() const;

	// create talbe.
	inline void create_table(
		IN const ObjectMapping& map,
		IN bool check_table_exist = true,
		IN const char* table = nullptr)
	{
		if (!check_table_exist || !has_table(map.get_table(table)))
		{
			std::string sql;
			map.get_create_table_sql(sql, &config(), table);
			execute_sql(sql.c_str());
		}
	}

	// save data to database.
	template<typename meta_t, typename object_set_t>
	inline uint64_t save_all(
		IN const object_set_t& set,
		IN const ObjectMapping& mapping,
		IN const char* table = nullptr)
	{
		uint64_t siz = 0;
		Transaction trans(*this);
		for (auto it = set.begin(); it != set.end(); ++it)
		{
			siz += save<meta_t>(*it, mapping, table);
		}
		trans.commit();
		return siz;
	}

	// save data to database.
	template<typename meta_t, typename object_set_t>
	inline uint64_t save_all(
		IN const object_set_t& set,
		IN const ObjectMapping& mapping,
		IN const eco::meta::Stamp s,
		IN const char* table = nullptr)
	{
		uint64_t siz = 0;
		Transaction trans(*this);
		for (auto it = set.begin(); it != set.end(); ++it)
		{
			siz += save<meta_t>(*it, mapping, s, table);
		}
		trans.commit();
		return siz;
	}

	// save data to database.
	template<typename meta_t, typename object_t>
	inline uint64_t save(
		IN const object_t& obj,
		IN const ObjectMapping& mapping,
		IN const eco::meta::Stamp stamp,
		IN const char* table = nullptr)
	{
		std::string sql;
		if (eco::meta::is_clean(stamp))		// origin object.
		{
			return 0;	// no need to save.
		}
		else if (eco::meta::is_insert(stamp))	// insert object.
		{
			mapping.get_insert_sql<meta_t>(sql, obj, table);
		}
		else if (eco::meta::is_update(stamp))	// update object.
		{
			mapping.get_update_sql<meta_t>(sql, obj, table);
		}
		else if (eco::meta::is_remove(stamp))	// delete object.
		{
			mapping.get_delete_sql<meta_t>(sql, obj, table);
		}
		else
		{
			ECO_THROW("save object failed, stamp is invalid.");
		}

		// execute sql to save data.
		uint64_t rows = execute_sql(sql.c_str());
		return rows;
	}

	// save data to database.
	template<typename meta_t, typename object_t>
	inline uint64_t save(
		IN const object_t& obj,
		IN const ObjectMapping& mapping,
		IN const char* table = nullptr)
	{
		meta_t meta;
		meta.attach(obj);
		uint64_t rows = save<meta_t>(obj, mapping, meta.stamp(), table);
		// update and insert object will reset stamp to original.
		eco::meta::clean(meta.stamp());
		return rows;
	}

	// insert data to database.
	template<typename meta_t, typename object_t>
	inline uint64_t insert(
		IN const object_t& obj,
		IN const ObjectMapping& mapping,
		IN const char* table = nullptr)
	{
		return save<meta_t>(obj, mapping, eco::meta::stamp_insert, table);
	}

	// remove object set by pk.
	template<typename meta_t, typename object_set_t>
	inline uint64_t insert_all(
		IN const object_set_t& obj_set,
		IN const ObjectMapping& mapping,
		IN const char* table = nullptr)
	{
		Transaction trans(*this);
		uint64_t siz = 0;
		for (auto it = obj_set.begin(); it != obj_set.end(); ++it)
		{
			siz += insert<meta_t>(*it, mapping, table);
		}
		trans.commit();
		return siz;
	}

	// update data to database.
	template<typename meta_t, typename object_t>
	inline uint64_t update(
		IN const object_t& obj,
		IN const ObjectMapping& mapping,
		IN const char* table = nullptr)
	{
		return save<meta_t>(obj, mapping, eco::meta::stamp_update, table);
	}

	// update object property value.
	template<typename meta_t, typename object_t>
	inline uint64_t update(
		IN object_t& obj,
		IN const char* prop,
		IN const char* value,
		IN const ObjectMapping& mapping,
		IN const char* table = nullptr)
	{
		auto* p = mapping.find(prop);
		if (!p) ECO_THROW("property isn't exist: ") << prop;

		meta_t meta;
		meta.attach(obj);
		// sql: "update set prop='value' where pk='v'"
		std::string sql("update ");
		sql.reserve(128);
		sql += mapping.get_table(table);
		sql += " set ";
		sql += p->field();
		sql += "='";
		sql += (value != nullptr) ? value : meta.get_value(prop, eco_db);
		sql += "'";
		// condition sql: "where pk='v'"
		std::string cond_sql;
		mapping.get_condition_sql(cond_sql, meta);
		sql += cond_sql;
		uint64_t rows = execute_sql(sql.c_str());
		if (value != nullptr && rows > 0) meta.set_value(prop, value);
		return rows;
	}
	template<typename meta_t, typename object_t>
	inline uint64_t update(
		IN object_t& obj,
		IN const std::string& prop,
		IN const std::string& value,
		IN const ObjectMapping& mapping,
		IN const char* table = nullptr)
	{
		return update<meta_t, object_t>(
			obj, prop.c_str(), value.c_str(), mapping, table);
	}
#ifndef ECO_NO_PROTOBUF
	template<typename meta_t, typename object_t>
	inline uint64_t update(
		IN object_t& obj,
		IN const eco::proto::Property& prop,
		IN const ObjectMapping& mapping,
		IN const char* table = nullptr)
	{
		return update<meta_t, object_t>(
			obj, prop.name(), prop.value(), mapping, table);
	}
#endif

	// update object property value set.
	template<typename meta_t, typename object_t>
	inline uint64_t update(
		IN object_t& obj,
		IN const ObjectMapping& mapping,
		IN const std::vector<std::string>& props,
		IN const char* table = nullptr)
	{
		meta_t meta;
		meta.attach(obj);
		// sql: "update set prop='value' where pk='v'"
		std::string sql("update ");
		sql.reserve(128);
		sql += mapping.get_table(table);
		sql += " set ";
		for (const std::string& it : props)
		{
			const PropertyMapping* p = mapping.find(it.c_str());
			if (!p) ECO_THROW("property isn't exist: ") < it;
			sql += p->field();
			sql += "='";
			sql += meta.get_value(p->property(), eco_db);
			sql += "',";
		}
		sql.pop_back();
		
		// condition sql: "where pk='v'"
		std::string cond_sql;
		mapping.get_condition_sql(cond_sql, meta);
		sql += cond_sql;
		uint64_t rows = execute_sql(sql.c_str());
		return rows;
	}

	// remove object by pk.
	inline uint64_t remove(
		IN const char* value,
		IN const ObjectMapping& mapping,
		IN const char* table = nullptr)
	{
		// sql: "delete table where pk='value'
		auto* p = mapping.find_pk();
		if (!p) ECO_THROW("pk property isn't exist.");
		std::string sql("delete from ");
		sql += mapping.get_table(table);
		sql += " where ";
		sql += p->field();
		sql += "='";
		sql += value;
		sql += "'";
		uint64_t rows = execute_sql(sql.c_str());
		return rows;
	}
	inline uint64_t remove(
		IN const uint64_t value,
		IN const ObjectMapping& mapping,
		IN const char* table = nullptr)
	{
		return remove(eco::Integer<uint64_t>(value).c_str(), mapping, table);
	}
	inline uint64_t remove(
		IN const std::string& value,
		IN const ObjectMapping& mapping,
		IN const char* table = nullptr)
	{
		return remove(value.c_str(), mapping, table);
	}

	// remove object set by pk.
	template<typename id_set_t>
	inline uint64_t remove_all(
		IN const id_set_t& set,
		IN const ObjectMapping& mapping,
		IN const char* table = nullptr)
	{
		Transaction trans(*this);
		uint64_t siz = 0;
		for (auto it = set.begin(); it != set.end(); ++it)
		{
			siz += remove(*it, mapping, table);
		}
		trans.commit();
		return siz;
	}

	// removed all data from table.
	inline uint64_t remove(
		IN const char* table,
		IN const std::string& cond_sql = std::string())
	{
		std::string sql("delete from ");
		sql += table;
		if (!cond_sql.empty())
		{
			sql += ' ';
			sql += cond_sql;
		}
		uint64_t rows = execute_sql(sql.c_str());
		return rows;
	}

	// read data from database.
	template<typename meta_t, typename object_t>
	inline bool read(
		OUT object_t& obj,
		IN  const ObjectMapping& mapping)
	{
		std::string sql;
		mapping.get_select_sql<meta_t>(sql, obj, mapping.get_table());

		Record rd;
		bool has = select(rd, sql.c_str());
		mapping.decode<meta_t>(obj, rd, eco_db);
		return has;
	}

	// read data from database.
	template<typename meta_t, typename object_t>
	inline bool read(
		OUT object_t& obj,
		IN  const ObjectMapping& mapping,
		IN  const char* cond_sql,
		IN  const char* table = nullptr,
		IN  const char lias_table = 0)
	{
		std::string sql;
		mapping.get_select_sql(sql, cond_sql, table, lias_table);

		Record rd;
		bool has = select(rd, sql.c_str());
		mapping.decode<meta_t>(obj, rd, eco_db);
		return has;
	}

	// save data to database.
	template<typename meta_t, typename object_set_t>
	inline void select(
		OUT object_set_t& obj_set,
		IN  const ObjectMapping& mapping,
		IN  const char* cond_sql = "",
		IN  const char* table = nullptr,
		IN  const char lias_table = 0)
	{
		std::string sql;
		mapping.get_select_sql(sql, cond_sql, table, lias_table);
		Recordset rd_set;
		select(rd_set, sql.c_str());
		mapping.decode_some<meta_t>(obj_set, rd_set, eco_db);
	}

	// save data to database.
	template<typename object_set_t, typename object_t>
	inline void select_join(
		OUT object_set_t& obj_set,
		IN  const JoinMapping<object_t>& join_map,
		IN  const char* cond_sql = "")
	{
		std::string sql;
		join_map.get_select_join_sql(sql, cond_sql);
		Recordset rd_set;
		select(rd_set, sql.c_str());
		
		// decode meta.
		join_map.decode_join(obj_set, rd_set, eco_db);
	}

	// is exist record in table.
	inline uint32_t has_record(
		IN const char* table_name,
		IN const char* cond_sql = "")
	{
		// format sql.
		std::string sql("select count(*) from ");
		sql += table_name;
		if (!eco::empty(cond_sql))
		{
			sql += ' ';
			sql += cond_sql;
		}

		// query sql.
		Record rd;
		select(rd, sql.c_str());
		uint32_t rows = eco::cast<uint32_t>(rd[0]);
		return rows;
	}

	// is exist object in table.
	template<typename meta_t, typename object_t>
	inline bool has_object(
		IN const object_t& obj,
		IN const ObjectMapping& mapping)
	{
		meta_t meta;
		meta.attach(obj);
		// condition sql: "where pk='v'"
		std::string cond_sql;
		mapping.get_condition_sql(cond_sql, meta);
		return has_record(mapping.get_table(), cond_sql.c_str());
	}

public:
	inline uint64_t get_max_id(
		IN const ObjectMapping& mapping,
		IN const char* table = nullptr)
	{
		// sql: "select max(id) from table".
		auto* pk = mapping.find_pk();
		if (!pk) ECO_THROW("get max id fail, pk isn't exist.");
		std::string sql("select max(");
		sql += pk->property();
		sql += ") from ";
		sql += mapping.get_table(table);

		// get max id.
		eco::Record rd;
		select(rd, sql.c_str());
		return eco::cast<uint64_t>(rd[0]);
	}
};


ECO_API eco::Database* create_database(IN const persist::SourceType v);
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif