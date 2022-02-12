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
#include <eco/Object.h>
#include <eco/rx/RxImpl.h>
#include <eco/persist/DatabaseConfig.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
enum FieldType
{
	field_int,
	field_bigint,
	field_char,
	field_char_array,
	field_varchar,
	field_double,
	field_text,
	field_blob,

	// database relative datatype.
	field_date_time,		// 2017-08-29 09:56:30
};


////////////////////////////////////////////////////////////////////////////////
enum
{
	constraint_pk		= 0x0001,		// primary key.
	constraint_fk		= 0x0002,		// foreign key.
	constraint_index	= 0x0004,		// index.
	constraint_unique	= 0x0008,		// unique index.
	constraint_nnull	= 0x0010,		// not null.
};
typedef uint16_t Constraint;


////////////////////////////////////////////////////////////////////////////////
class PropertyMapping
{
public:
	explicit inline PropertyMapping(IN uint32_t index = 0)
		: m_field_index(index), m_constraint(eco::value_none)
	{}

	inline bool operator==(IN const char* field_name) const
	{
		return m_field_name == field_name;
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
	inline PropertyMapping& prop(IN const char* prop_name)
	{
		m_prop_name = prop_name;
		return *this;
	}

	inline PropertyMapping& pk(IN bool is = true)
	{
		eco::set(m_constraint, constraint_pk, is);
		return *this;
	}

	inline PropertyMapping& fk(IN bool is = true)
	{
		eco::set(m_constraint, constraint_fk, is);
		return *this;
	}

	inline PropertyMapping& index(IN bool is = true)
	{
		eco::set(m_constraint, constraint_index, is);
		return *this;
	}

	inline PropertyMapping& unique(IN bool is = true)
	{
		eco::set(m_constraint, constraint_unique, is);
		return *this;
	}

	inline PropertyMapping& not_null(IN bool is = true)
	{
		eco::set(m_constraint, constraint_nnull, is);
		return *this;
	}

	inline PropertyMapping& int_type()
	{
		m_field_type = field_int;
		m_field_size = 0;
		return *this;
	}
	inline PropertyMapping& int32()
	{
		return int_type();
	}

	inline PropertyMapping& big_int()
	{
		m_field_type = field_bigint;
		m_field_size = 0;
		return *this;
	}
	inline PropertyMapping& int64()
	{
		return big_int();
	}

	inline PropertyMapping& double_type()
	{
		m_field_type = field_double;
		m_field_size = 0;
		return *this;
	}

	inline PropertyMapping& char_type(
		IN const uint32_t field_size = 0)
	{
		m_field_size = field_size;
		m_field_type = field_size == 0 ? field_char : field_char_array;
		return *this;
	}

	inline PropertyMapping& varchar(
		IN const uint32_t field_size)
	{
		m_field_type = field_varchar;
		m_field_size = field_size;
		return *this;
	}

	inline PropertyMapping& vchar_small()
	{
		m_field_type = field_varchar;
		m_field_size = 255;
		return *this;
	}

	inline PropertyMapping& vchar_middle()
	{
		m_field_type = field_varchar;
		m_field_size = 8192;	// detail in mysql limited.
		return *this;
	}

	inline PropertyMapping& text()
	{
		m_field_type = field_text;
		m_field_size = 65535;		// sqlite/mysql defined.
		return *this;
	}

	inline PropertyMapping& blob()
	{
		m_field_type = field_blob;
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
		m_field_type = field_date_time;
		m_field_size = 0;
		return *this;
	}

	inline PropertyMapping& field_index(uint32_t i)
	{
		m_field_index = i;
		return *this;
	}

public:
	inline const char* field() const
	{
		return m_field_name.c_str();
	}

	inline const char* property() const
	{
		return m_prop_name.c_str();
	}
	inline operator const char* () const
	{
		return m_prop_name.c_str();
	}
	inline operator uint32_t() const
	{
		return m_field_index;
	}

	inline bool is_property(IN const char* prop) const
	{
		return m_prop_name == prop;
	}

	inline bool is_pk() const
	{
		return eco::has(m_constraint, eco::constraint_pk);
	}

	inline bool is_fk() const
	{
		return eco::has(m_constraint, eco::constraint_fk);
	}

	inline bool is_index() const
	{
		return eco::has(m_constraint, eco::constraint_index);
	}

	inline bool is_unique() const
	{
		return eco::has(m_constraint, eco::constraint_unique);
	}

	inline bool is_not_null() const
	{
		return eco::has(m_constraint, eco::constraint_nnull);
	}

	inline bool has_constraint(IN uint16_t v) const
	{
		return eco::has(m_constraint, v);
	}

	inline uint32_t field_index() const
	{
		return m_field_index;
	}

	inline uint32_t field_type() const
	{
		return m_field_type;
	}

	inline static void get_field_type_sql(
		OUT std::string& field_sql,
		IN  uint32_t field_type,
		IN  uint32_t field_size)
	{
		switch (field_type)
		{
		case field_int:
			field_sql = "INT";
			break;
		case field_bigint:
			field_sql = "BIGINT";
			break;
		case field_char:
			field_sql = "CHAR";
			break;
		case field_char_array:
			field_sql = "CHAR(";
			field_sql += eco::cast(field_size);
			field_sql += ")";
			break;
		case field_varchar:
			field_sql = "VARCHAR(";
			field_sql += eco::cast(field_size);
			field_sql += ")";
			break;
		case field_text:
			field_sql = "TEXT";
			break;
		case field_blob:
			field_sql = "BLOB";
			break;
		case field_double:
			field_sql = "DOUBLE";
			break;
		}
	}

	inline std::string get_field_type_sql(IN  DatabaseConfig* cfg) const
	{
		// standard field sql.
		std::string temp;
		get_field_type_sql(temp, m_field_type, m_field_size);

		// special field by database.
		if (temp.empty() && cfg != nullptr)
		{
			cfg->get_field_type_sql(temp, m_field_type, m_field_size);
		}
		if (temp.empty())
		{
			ECO_THROW("invalid database field type") < m_field_type;
		}
		return temp;
	}

	inline std::string index_name() const
	{
		return std::string("_index_") + m_field_name;
	}

	inline bool get_index_sql(OUT std::string& sql) const
	{
		if (is_index())
		{
			sql += "INDEX ";
		}
		else if (is_unique())
		{
			sql += "UNIQUE INDEX ";
		}
		else
		{
			return false;
		}

		if (!sql.empty())
		{
			sql += index_name();
			sql += " (";
			sql += m_field_name;
			sql += ")";
		}
		return true;
	}

////////////////////////////////////////////////////////////////////////////////
private:
	std::string		m_prop_name;	// data object property.
	std::string		m_field_name;	// db field name.
	uint32_t		m_field_index;	// db field index.
	FieldType		m_field_type;	// field type.
	uint32_t		m_field_size;	// field size.
	Constraint		m_constraint;	// field constraint.
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif