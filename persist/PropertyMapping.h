#ifndef ECO_PROPERTY_MAPPING_H
#define ECO_PROPERTY_MAPPING_H
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
#include <eco/Type.h>
#include <eco/Implement.h>
#include <eco/persist/DatabaseConfig.h>


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
enum DataType
{
	type_int,
	type_bigint,
	type_char,
	type_char_array,
	type_varchar,
	type_double,
	type_text,
	type_blob,

	// database relative datatype.
	type_date_time,		// 2017-08-29 09:56:30
};


////////////////////////////////////////////////////////////////////////////////
enum
{
	constraint_pk		= 0x0001,		// primary key.
	constraint_fk		= 0x0002,		// foreign key.
};
typedef uint16_t DataConstraint;


////////////////////////////////////////////////////////////////////////////////
class PropertyMapping
{
public:
	PropertyMapping() : m_constraint(eco::value_none), m_field_index(0)
	{}

	inline bool operator==(IN const char* field_name) const
	{
		return strcmp(m_field_name, field_name) == 0;
	}

	inline PropertyMapping& field(IN const char* field)
	{
		m_field_name = field;
		return *this;
	}

	inline PropertyMapping& property(IN const char* prop_name)
	{
		m_prop_name = prop_name;
		return *this;
	}

	inline PropertyMapping& pk(IN const bool is)
	{
		eco::set(m_constraint, constraint_pk, is);
		return *this;
	}

	inline PropertyMapping& fk(IN const bool is)
	{
		eco::set(m_constraint, constraint_fk, is);
		return *this;
	}

	inline PropertyMapping& int_type()
	{
		m_field_type = type_int;
		m_field_size = 0;
		return *this;
	}

	inline PropertyMapping& big_int()
	{
		m_field_type = type_bigint;
		m_field_size = 0;
		return *this;
	}

	inline PropertyMapping& double_type()
	{
		m_field_type = type_double;
		m_field_size = 0;
		return *this;
	}

	inline PropertyMapping& char_type(
		IN const uint32_t field_size = 0)
	{
		m_field_size = field_size;
		m_field_type = field_size == 0 ? type_char : type_char_array;
		return *this;
	}

	inline PropertyMapping& varchar(
		IN const uint32_t field_size)
	{
		m_field_type = type_varchar;
		m_field_size = field_size;
		return *this;
	}

	inline PropertyMapping& vchar_small()
	{
		m_field_type = type_varchar;
		m_field_size = 255;
		return *this;
	}

	inline PropertyMapping& vchar_middle()
	{
		m_field_type = type_varchar;
		m_field_size = 8192;	// detail in mysql limited.
		return *this;
	}

	inline PropertyMapping& text()
	{
		m_field_type = type_text;
		m_field_size = 65535;		// sqlite/mysql defined.
		return *this;
	}

	inline PropertyMapping& blob()
	{
		m_field_type = type_blob;
		m_field_size = 65535;		// sqlite/mysql defined.
		return *this;
	}

	inline PropertyMapping& date()
	{
		return char_type(10);
	}

	inline PropertyMapping& time()
	{
		return char_type(8);
	}

	inline PropertyMapping& date_time()
	{
		return char_type(19);
	}

	inline PropertyMapping& date_time_t()
	{
		m_field_type = type_date_time;
		m_field_size = 0;
		return *this;
	}

public:
	inline const char* get_field() const
	{
		return m_field_name;
	}

	inline const char* get_property() const
	{
		return m_prop_name;
	}

	inline bool is_property(IN const char* prop) const
	{
		return strcmp(m_prop_name, prop) == 0;
	}

	inline bool is_pk() const
	{
		return eco::has(m_constraint, eco::constraint_pk);
	}

	inline bool is_fk() const
	{
		return eco::has(m_constraint, eco::constraint_fk);
	}

	inline bool has_constraint(IN const uint16_t v) const
	{
		return eco::has(m_constraint, v);
	}

	inline uint32_t get_field_index() const
	{
		return m_field_index;
	}

	inline uint32_t get_field_type() const
	{
		return m_field_type;
	}

	inline static void get_field_type_sql(
		OUT std::string& field_sql,
		IN  const uint32_t field_type,
		IN  const uint32_t field_size)
	{
		switch (field_type)
		{
		case type_int:
			field_sql = "INT";
			break;
		case type_bigint:
			field_sql = "BIGINT";
			break;
		case type_char:
			field_sql = "CHAR";
			break;
		case type_char_array:
			field_sql = "CHAR(";
			field_sql += eco::cast<std::string>(field_size);
			field_sql += ")";
			break;
		case type_varchar:
			field_sql = "VARCHAR(";
			field_sql += eco::cast<std::string>(field_size);
			field_sql += ")";
			break;
		case type_text:
			field_sql = "TEXT";
			break;
		case type_blob:
			field_sql = "BLOB";
			break;
		case type_double:
			field_sql = "DOUBLE";
			break;
		}
	}

	inline std::string get_field_type_sql(IN  DatabaseConfig* cfg) const
	{
		std::string temp;
		get_field_type_sql(temp, m_field_type, m_field_size);
		if (temp.empty() && cfg != nullptr)
		{
			cfg->get_field_type_sql(temp, m_field_type, m_field_size);
		}
		if (temp.empty())
		{
			EcoThrow << "invalid database field type: " << m_field_type;
		}
		return temp;
	}

////////////////////////////////////////////////////////////////////////////////
private:
	const char*		m_prop_name;	// data object property.
	const char*		m_field_name;	// db field name.
	uint32_t		m_field_index;	// db field index.
	uint32_t		m_field_type;	// field type.
	uint32_t		m_field_size;	// field size.
	uint16_t		m_constraint;	// field constraint.
};


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif