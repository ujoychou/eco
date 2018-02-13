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
class JoinMeta
{
public:
	inline JoinMeta(const ObjectMapping* v) : m_map(v)
	{}

	virtual void* create() = 0;

	virtual void set_value(
		IN const char* prop,
		IN const char* value,
		IN const char* view = nullptr) = 0;

	virtual eco::meta::Timestamp& timestamp() = 0;

	virtual void attach(void* object) = 0;

public:
	const ObjectMapping* m_map;
};


////////////////////////////////////////////////////////////////////////////////
template<typename meta_t>
class JoinMetaImpl : public JoinMeta
{
public:
	JoinMetaImpl(const ObjectMapping& map) : JoinMeta(&map)
	{}

	virtual void* create() override
	{
		return m_meta.create();
	}

	virtual void set_value(
		IN const char* prop,
		IN const char* value,
		IN const char* view = nullptr) override
	{
		m_meta.set_value(prop, value, view);
	}

	virtual eco::meta::Timestamp& timestamp() override
	{
		return m_meta.timestamp();
	}

	virtual void attach(IN void* obj) override
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
		eco::release(m_join_meta);
	}

	// get object map table alias.
	inline static const char get_table_alias(IN const uint32_t i)
	{
		return 'A' + i;
	}

	// add join object and meta.
	template<typename meta_t>
	inline void add_join(IN const ObjectMapping& map)
	{
		m_join_meta.push_back(new JoinMetaImpl<meta_t>(map));
	}

	// add main object and meta.
	template<typename meta_t>
	inline void add_main(IN const ObjectMapping& map)
	{
		m_main_meta.reset(new JoinMetaImpl<meta_t>(map));
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
			get_map(i)->get_join_sql(sql, get_table_alias(i + 1),
				get_map(i)->get_table(), *get_main_table());
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
			void* vobj = m_main_meta->create();
			object_set_t::value_type obj((object_t*)vobj);

			// set main table object property value.
			uint32_t field = 0;
			m_main_meta->attach(vobj);
			auto m = get_main_table()->get_map().begin();
			for (; m != get_main_table()->get_map().end(); ++m)
			{
				m_main_meta->set_value(m->get_property(),
					record_set[r][field++], view);
			}

			// set join table object property value.
			for (auto j = m_join_meta.begin(); j != m_join_meta.end(); ++j)
			{
				JoinMeta* join_meta = *j;
				join_meta->attach(vobj);
				auto it = join_meta->m_map->get_map().begin();
				for (; it != join_meta->m_map->get_map().end(); ++it)
				{
					if (get_main_table()->find_property(it->get_property(), 
						eco::constraint_fk | eco::constraint_pk))
					{
						continue;
					}
					join_meta->set_value(
						it->get_property(), record_set[r][field++], view);
				}
			}
			eco::meta::clear(m_main_meta->timestamp());
			obj_set.push_back(obj);
		}
	}

////////////////////////////////////////////////////////////////////////////////
private:
	std::auto_ptr<JoinMeta>	m_main_meta;
	std::vector<JoinMeta*>  m_join_meta;
};


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif