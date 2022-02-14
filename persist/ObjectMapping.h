#ifndef ECO_OBJECT_MAPPING_H
#define ECO_OBJECT_MAPPING_H
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
#include <eco/Object.h>
#include <eco/meta/Meta.h>
#include <eco/persist/PropertyMapping.h>
#include <eco/persist/Recordset.h>
#include <vector>
#include <algorithm>


ECO_NS_BEGIN(eco);
const char* const eco_db = "eco_db";	// using by meta, get/set object value.
////////////////////////////////////////////////////////////////////////////////
class ObjectMapping : public eco::Object<ObjectMapping>
{
public:
	/*@ constructor. */
	inline ObjectMapping() : m_id(nullptr)
	{}

	/*@ destructor. */
	inline ~ObjectMapping()
	{}

/////////////////////////////////////////////////////////////////////META OBJECT
public:
	inline const char* get_table(IN const char* table) const
	{
		return (table != nullptr) ? table : m_table.c_str();
	}

	inline static void get_field_alias(
		OUT std::string& field_sql, IN const char table_alias)
	{
		if (table_alias != 0)
		{
			field_sql += table_alias;
			field_sql += '.';
		}
	}

	// throw error.
	inline void throw_error() const
	{
		std::string temp("property map is empty: ");
		temp += m_id;
		throw std::logic_error(temp.c_str());
	}

	// get create table sql.
	inline void get_create_table_sql(
		OUT std::string& sql,
		IN  DatabaseConfig* cfg,
		IN  const char* table) const
	{
		if (m_prop_map.empty())
		{
			throw_error();
		}

		// create table fields.
		sql = "CREATE TABLE ";
		sql += get_table(table);
		sql += '(';
		auto it = m_prop_map.begin();
		for (int i = 0; it != m_prop_map.end(); ++it, ++i)
		{
			sql += it->field();
			sql += " ";
			sql += it->get_field_type_sql(cfg);
			sql += ",";
		}

		// primary key: "PRIMARY KEY (pk1, pk2)"
		std::string temp("PRIMARY KEY (");
		for (auto it = m_prop_map.begin(); it != m_prop_map.end(); ++it)
		{
			if (it->is_pk())
			{
				temp += it->field();
				temp += ",";
			}
		}
		if (temp.back() != '(')		// not empty.
		{
			temp.back() = ')';
			sql += temp;
			sql += ',';
		}

		// index: unique / normal.
		for (auto it = m_prop_map.begin(); it != m_prop_map.end(); ++it)
		{
			if (cfg->get_index_sql(sql, *it))
				sql += ',';
		}
		sql.back() = ')';
	}

	template<typename meta_t, typename object_t>
	void get_insert_sql(
		OUT std::string& sql,
		IN  const object_t& obj,
		IN  const char* table) const
	{
		meta_t meta;
		meta.attach(obj);

		// mapping is empty, return a empty sql.
		if (m_prop_map.empty())
		{
			sql.clear();
			throw_error();
		}

		// construct insert sql.
		std::string field_sql;
		std::string value_sql;
		for (auto it = m_prop_map.begin(); it != m_prop_map.end(); ++it)
		{
			field_sql += it->field();
			field_sql += ',';
			value_sql += '\'';
			value_sql += meta.get_value(*it, eco_db);
			value_sql += "',";
		}
		// remove end ','.
		field_sql.resize(field_sql.size() - 1);
		value_sql.resize(value_sql.size() - 1);

		// sql format: 
		// insert into table(field1,field2,...)values('value1','value2',...)
		sql.reserve(field_sql.size() + value_sql.size() + 50);
		sql = "insert into ";
		sql += get_table(table);
		sql += " (";
		sql += field_sql;
		sql += ") values(";
		sql += value_sql;
		sql += ")";
	}

	template<typename meta_t, typename object_t>
	void get_update_sql(
		OUT std::string& sql,
		IN  const object_t& obj,
		IN  const char* table) const
	{
		meta_t meta;
		meta.attach(obj);

		// mapping is empty, return a empty sql.
		if (m_prop_map.empty())
		{
			sql.clear();
			throw_error();
		}

		// sql format: 
		// update table set field1='value1', field2='value2',... where pk1=v1 and ...
		sql.reserve(256);
		sql = "update ";
		sql += get_table(table);
		sql += " set ";
		std::string cond_sql;
		get_condition_sql(cond_sql, meta);
		for (auto it = m_prop_map.begin(); it != m_prop_map.end(); ++it)
		{
			if (!it->is_pk())
			{
				sql += it->field();
				sql += "='";
				sql += meta.get_value(*it, eco_db);
				sql += "',";
			}
		}
		sql.resize(sql.size() - 1);
		sql += cond_sql;
	}

	template<typename meta_t, typename object_t>
	inline void get_delete_sql(
		OUT std::string& sql,
		IN  const object_t& obj,
		IN  const char* table) const
	{
		meta_t meta;
		meta.attach(obj);

		// sql format: 
		// delete from table where field1='value1' and field2='value2',...
		std::string cond_sql;
		get_condition_sql(cond_sql, meta);
		sql.reserve(64);
		sql = "delete from ";
		sql += get_table(table);
		sql += ' ';
		sql += cond_sql;
	}

	template<typename meta_t, typename object_t>
	inline void get_select_sql(
		OUT std::string& sql,
		IN  const object_t& obj,
		IN  const char* table) const
	{
		meta_t meta;
		meta.attach(obj);

		// mapping is empty, return a empty sql.
		if (m_prop_map.empty())
		{
			sql.clear();
			throw_error();
		}

		// construct condition sql.
		std::string cond_sql;
		get_condition_sql(cond_sql, meta);
		get_select_sql(sql, cond_sql.c_str(), table, 0);
	}

	inline void get_select_sql(
		OUT std::string& sql,
		IN  const char* cond_sql,
		IN  const char* table,
		IN  const char table_lias) const
	{
		// mapping is empty, return a empty sql.
		sql = ("select ");
		get_field_sql(sql, table_lias);
		sql.resize(sql.size() - 1);		// remove end ','.

		// sql format: 
		// select field1,field2,... from table where ...
		sql += " from ";
		sql += get_table(table);
		if (table_lias != 0)
		{
			sql += " as ";
			sql += table_lias;
		}
		if (!eco::empty(cond_sql))
		{
			sql += ' ';
			sql += cond_sql;
		}
	}

public:
	template<typename meta_t>
	void get_condition_sql(
		OUT std::string& cond_sql,
		IN  meta_t& meta) const
	{
		// sql format: 
		// where field1='value1' and field2='value2',...
		cond_sql.reserve(64);
		for (auto it = m_prop_map.begin(); it != m_prop_map.end(); ++it)
		{
			if (it->is_pk())
			{
				cond_sql += cond_sql.empty() ? " where " : " and ";
				cond_sql += it->field();
				cond_sql += "='";
				cond_sql += meta.get_value(*it, eco_db);
				cond_sql += "'";
			}
		}
		if (cond_sql.empty())
		{
			throw_error();
		}
	}

	// get select field and select join sql.
	inline void get_field_sql(
		OUT std::string& field_sql,
		IN  const char table_lias,
		IN  const ObjectMapping* main_table = nullptr) const
	{
		// mapping is empty, return a empty sql.
		if (m_prop_map.empty())
		{
			throw_error();
		}

		// sql format: A.field1,A.field2,...
		field_sql.reserve(256);
		for (auto it = m_prop_map.begin(); it != m_prop_map.end(); ++it)
		{
			// not main table pk.
			if (main_table == nullptr || !main_table->find(
				it->property(), constraint_pk | constraint_fk))
			{
				get_field_alias(field_sql, table_lias);
				field_sql += it->field();
				field_sql += ',';
			}
		}
	}

	// get join sql with main table.
	inline void get_join_sql(
		OUT std::string& join_sql,
		IN  const char table_alias,
		IN  const char* table_name,
		IN  const char* join_mode,
		IN  const ObjectMapping& main_table) const
	{
		// sql format: inner join YYY B on B.b1=A.a1
		std::string cond_sql;
		cond_sql.reserve(64);
		for (auto it = m_prop_map.begin(); it != m_prop_map.end(); ++it)
		{
			auto* main_prop = main_table.find(it->property(),
				constraint_pk | constraint_fk);
			if (main_prop != nullptr)
			{
				if (!cond_sql.empty())
					cond_sql += " and ";
				cond_sql += table_alias;
				cond_sql += '.';
				cond_sql += it->field();
				cond_sql += "=A.";
				cond_sql += main_prop->field();
			}
		}

		if (!cond_sql.empty())
		{
			join_sql += ' ';
			join_sql += join_mode;
			join_sql += ' ';
			join_sql += table_name;
			join_sql += " as ";
			join_sql += table_alias;
			join_sql += " on ";
			join_sql += cond_sql;
		}// end if.
	}

public:
	// decode object from dataset.
	template<typename meta_t, typename object_t>
	inline void decode(
		OUT object_t& obj,
		IN  const Record& record,
		IN  const char* view) const
	{
		meta_t meta;
		meta.attach(obj);

		if (m_prop_map.empty())			// property size is empty.
		{
			throw_error();
		}
		if (record.size() == 0)			// data size is empty.
		{
			return ;
		}

		// set object property value.
		auto it = m_prop_map.begin();
		for (int i=0; it!=m_prop_map.end(); ++it, ++i)
		{
			meta.set_value(*it, record[i], view);
		}
		eco::meta::clean(meta.stamp());
	}

	// decode object set from data sheet.
	template<typename meta_t, typename object_set_t>
	void decode_some(
		OUT object_set_t& obj_set,
		IN  const Recordset& record_set,
		IN  const char* view) const
	{
		if (m_prop_map.empty())
		{
			throw_error();
		}

		meta_t meta;
		for (size_t oi = 0; oi < record_set.size(); ++oi)
		{
			// create object.
			typename object_set_t::value_type obj;
			meta.make_attach(obj);

			// set object property value.
			auto it = m_prop_map.begin();
			for (int i = 0; it != m_prop_map.end(); ++it, ++i)
			{
				meta.set_value(*it, record_set[oi][i], view);
			}
			eco::meta::clean(meta.stamp());
			obj_set.push_back(obj);
		}
	}

////////////////////////////////////////////////////////////////////////////////
public:
	// reserve property mapping item capacity.
	inline ObjectMapping& reserve(IN const uint32_t siz)
	{
		m_prop_map.reserve(siz);
		return *this;
	}

	// Set database persist identity.
	inline ObjectMapping& id(IN const char* id)
	{
		m_id = id;
		return *this;
	}

	// Set database table name.
	inline ObjectMapping& table(IN const char* v)
	{
		m_table = v;
		return *this;
	}
	inline ObjectMapping& table(IN const std::string& v)
	{
		m_table = v;
		return *this;
	}

	// Get database table adapter identity.
	inline const char* get_id() const
	{
		return m_id;
	}

	// Get database table name.
	inline const char* get_table() const
	{
		return m_table.c_str();
	}

	// Add a field adapter.
	inline PropertyMapping& add()
	{
		uint32_t size = static_cast<uint32_t>(m_prop_map.size());
		m_prop_map.push_back(PropertyMapping(size));
		return m_prop_map.back();
	}

	// Remove a field.
	inline void erase(IN const char* prop)
	{
		for (auto it = m_prop_map.begin(); it != m_prop_map.end(); ++it)
		{
			if (strcmp(it->property(), prop) == 0)
			{
				m_prop_map.erase(it); break;
			}
		}
	}

	// Find a field adapter.
	inline const PropertyMapping* find_field(IN const char* field) const
	{
		auto it = std::find(m_prop_map.begin(), m_prop_map.end(), field);
		return (it != m_prop_map.end()) ? &*it : nullptr;
	}
	inline PropertyMapping* find_field_n(
		IN const char* field, IN const size_t size) const
	{
		for (auto it = m_prop_map.begin(); it != m_prop_map.end(); ++it)
		{
			if (strncmp(it->field(), field, size) == 0)
				return (PropertyMapping*)&*it;
		}
		return nullptr;
	}

	// find pk property mapping.
	inline PropertyMapping* find(
		IN const char* prop,
		IN const uint16_t constraint = 0) const
	{
		for (auto it = m_prop_map.begin(); it != m_prop_map.end(); ++it)
		{
			if ((constraint == 0 || it->has_constraint(constraint)) &&
				strcmp(it->property(), prop) == 0)
				return (PropertyMapping*)&*it;
		}
		return nullptr;
	}
	inline PropertyMapping* find_n(
		IN const char* prop,
		IN const size_t size,
		IN const uint16_t constraint = 0) const
	{
		for (auto it = m_prop_map.begin(); it != m_prop_map.end(); ++it)
		{
			if ((constraint == 0 || it->has_constraint(constraint)) &&
				strncmp(it->property(), prop, size) == 0)
				return (PropertyMapping*)&*it;
		}
		return nullptr;
	}

	// get pk property
	inline const PropertyMapping* find_pk() const
	{
		for (auto it = m_prop_map.begin(); it != m_prop_map.end(); ++it)
		{
			if (it->is_pk()) return &*it;
		}
		return nullptr;
	}

	// get pk filed count
	inline int get_pk_count() const
	{
		int pk_count = 0;
		for (auto it = m_prop_map.begin(); it != m_prop_map.end(); ++it)
		{
			if (it->is_pk())
				++pk_count;
		}
		return pk_count;
	}

	// get property mapping list.
	inline std::vector<PropertyMapping>& get_map()
	{
		return m_prop_map;
	}
	inline const std::vector<PropertyMapping>& map() const
	{
		return m_prop_map;
	}

	// copy this object.
	inline void copy(IN const ObjectMapping& obj_map)
	{
		m_prop_map = obj_map.m_prop_map;
		m_id = obj_map.m_id;
		m_table = obj_map.m_table;
	}
	inline void copy_property(IN const ObjectMapping& obj_map)
	{
		m_prop_map = obj_map.m_prop_map;
	}

////////////////////////////////////////////////////////////////////////////////
private:
	const char* m_id;
	std::string m_table;
	std::vector<PropertyMapping> m_prop_map;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif