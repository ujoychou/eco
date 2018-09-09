#ifndef ECO_OBJECT_MAPPING_SET_H
#define ECO_OBJECT_MAPPING_SET_H
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
#include <eco/persist/ObjectMapping.h>
#include <eco/meta/Meta.h>


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
enum
{
	// 4bit: join option
	option_join_left		= 0x00000001,
	option_join_right		= 0x00000002,
	option_join_inner		= 0x00000004,
};
typedef uint32_t JoinOption;


////////////////////////////////////////////////////////////////////////////////
inline void set_join(IN JoinOption& opt, IN const uint32_t v)
{
	opt >>= 4;
	opt <<= 4;
	opt |= v;
}


////////////////////////////////////////////////////////////////////////////////
inline const char* get_join(IN const JoinOption v)
{
	if (eco::has(v, option_join_inner))
		return "inner join";
	else if (eco::has(v, option_join_left))
		return "left join";
	else if (eco::has(v, option_join_right))
		return "right join";
	return "inner join";
}


////////////////////////////////////////////////////////////////////////////////
template<typename object_t>
class JoinMeta
{
public:
	inline JoinMeta(const ObjectMapping* v)
		: m_map(v), m_option(option_join_inner)
	{}

	virtual object_t create() = 0;

	virtual void set_value(
		IN const PropertyMapping& prop,
		IN const char* value,
		IN const char* view = nullptr) = 0;

	virtual eco::meta::Stamp& stamp() = 0;

	virtual void attach(IN object_t& object) = 0;

public:
	JoinOption m_option;
	const ObjectMapping* m_map;
};


////////////////////////////////////////////////////////////////////////////////
template<typename meta_t, typename object_t>
class JoinMetaImpl : public JoinMeta<object_t>
{
public:
	JoinMetaImpl(const ObjectMapping& map) : JoinMeta<object_t>(&map)
	{}

	virtual object_t create() override
	{
		return m_meta.create();
	}

	virtual void set_value(
		IN const PropertyMapping& prop,
		IN const char* value,
		IN const char* view = nullptr) override
	{
		m_meta.set_value(prop, value, view);
	}

	virtual eco::meta::Stamp& stamp() override
	{
		return m_meta.stamp();
	}

	virtual void attach(IN object_t& obj) override
	{
		m_meta.attach(obj);
	}

public:
	meta_t m_meta;
};


////////////////////////////////////////////////////////////////////////////////
template<typename object_t>
class JoinMapping
{
	ECO_OBJECT(JoinMapping);
public:
	inline JoinMapping()
	{}

	inline ~JoinMapping()
	{
		eco::release<JoinMeta<object_t>>(m_join_meta);
	}

	// get object map table alias.
	inline static const char get_table_alias(IN const uint32_t i)
	{
		return 'A' + i;
	}

	// add join object and meta.
	template<typename meta_t>
	inline void join(IN const ObjectMapping& map)
	{
		m_join_meta.push_back(new JoinMetaImpl<meta_t, object_t>(map));
		set_join(m_join_meta.back()->m_option, option_join_inner);
	}
	template<typename meta_t>
	inline void join_left(IN const ObjectMapping& map)
	{
		m_join_meta.push_back(new JoinMetaImpl<meta_t, object_t>(map));
		set_join(m_join_meta.back()->m_option, option_join_left);
	}
	template<typename meta_t>
	inline void join_right(IN const ObjectMapping& map)
	{
		m_join_meta.push_back(new JoinMetaImpl<meta_t, object_t>(map));
		set_join(m_join_meta.back()->m_option, option_join_right);
	}

	// add main object and meta.
	template<typename meta_t>
	inline void main(IN const ObjectMapping& map)
	{
		m_main_meta.reset(new JoinMetaImpl<meta_t, object_t>(map));
	}

	// get main object map table.
	inline const ObjectMapping* get_main_table() const
	{
		return m_main_meta->m_map;
	}

	// get join object map size.
	inline const size_t size() const
	{
		return m_join_meta.size();
	}
	inline const ObjectMapping* operator[](IN const size_t i) const
	{
		return m_join_meta[i]->m_map;
	}
	inline const ObjectMapping* get_map(IN const size_t i) const
	{
		return m_join_meta[i]->m_map;
	}

	inline void get_select_join_sql(
		OUT std::string& sql, 
		IN  const char* cond_sql) const
	{
		// select A.a1, B.b1 from XXX A inner join YYY B on A.a1=B.b1
		sql = ("select ");

		// field sql.
		get_main_table()->get_field_sql(sql, get_table_alias(0));
		for (uint32_t i = 0; i < size(); ++i)
		{
			get_map(i)->get_field_sql(
				sql, get_table_alias(i + 1), get_main_table());
		}
		sql.resize(sql.size() - 1);		// remove end ','
		sql += " from ";
		sql += get_main_table()->get_table();
		sql += " as ";
		sql += get_table_alias(0);
		sql += " ";

		// inner join sql.
		for (uint32_t i = 0; i < size(); ++i)
		{
			get_map(i)->get_join_sql(
				sql, get_table_alias(i + 1),
				get_map(i)->get_table(),
				get_join(m_join_meta[i]->m_option),
				*get_main_table());
		}

		// where sql.
		if (!eco::empty(cond_sql))
		{
			sql += ' ';
			sql += cond_sql;
		}
	}

	// decode object set from data sheet.
	template<typename object_set_t>
	void decode_join(
		OUT object_set_t& obj_set,
		IN  const Recordset& record_set,
		IN  const char* view) const
	{
		for (size_t r = 0; r < record_set.size(); ++r)
		{
			// create object.
			object_set_t::value_type obj(m_main_meta->create());

			// set main table object property value.
			uint32_t field = 0;
			m_main_meta->attach(obj);
			auto m = get_main_table()->get_map().begin();
			for (; m != get_main_table()->get_map().end(); ++m)
			{
				m_main_meta->set_value(*m, record_set[r][field++], view);
			}

			// set join table object property value.
			for (auto j = m_join_meta.begin(); j != m_join_meta.end(); ++j)
			{
				JoinMeta<object_t>* join_meta = *j;
				join_meta->attach(obj);
				auto it = join_meta->m_map->get_map().begin();
				for (; it != join_meta->m_map->get_map().end(); ++it)
				{
					if (get_main_table()->find_property(it->get_property(), 
						eco::constraint_fk | eco::constraint_pk))
					{
						continue;
					}
					join_meta->set_value(*it, record_set[r][field++], view);
				}
			}
			eco::meta::clean(m_main_meta->stamp());
			obj_set.push_back(obj);
		}
	}

////////////////////////////////////////////////////////////////////////////////
private:
	std::auto_ptr<JoinMeta<object_t>>	m_main_meta;
	std::vector<JoinMeta<object_t>*>  m_join_meta;
};


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif