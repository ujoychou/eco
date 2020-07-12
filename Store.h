#ifndef ECO_STORE_H
#define ECO_STORE_H
/*******************************************************************************
@ name
convert types.

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-01-15.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Export.h>
#include <eco/Memory.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<typename id_t, typename key_t>
class IndexKey
{
public:
	id_t	m_id;
	key_t	m_key;

	inline IndexKey(const id_t& id, const key_t& key) : m_id(id), m_key(key)
	{}

	inline bool operator<(const IndexKey& key) const
	{
		if (m_key < key.m_key) return true;
		if (m_key > key.m_key) return false;
		return (m_id < key.m_id);
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename id_t, typename object_t>
class Index
{
	ECO_OBJECT(Index);
public:
	inline Index() {};

	// setup index by data.
	virtual void setup(IN const object_t& data);

	// remove index by data.
	virtual void remove(IN const object_t& data);

	// get data id by index.
	virtual id_t get_id(IN const object_t& data) const;

	// get index last object id.
	virtual id_t get_last() const;
};


////////////////////////////////////////////////////////////////////////////////
template<typename id_t, typename key_t, typename object_t, typename meta_t>
class IndexT : public Index<id_t, object_t>
{
public:
	typedef IndexKey<id_t, key_t> Key;
	std::set<Key> m_keys;
	std::string   m_name;

	// setup index by data.
	virtual void setup(IN const object_t& data)
	{
		Key key;
		key.m_id = meta_t::get_id(data);
		meta_t::get_key(key.m_key, data);
		m_keys.insert(key);
	}

	// remove index by data.
	virtual void remove(IN const object_t& data)
	{
		Key key;
		key.m_id = meta_t::get_id(data);
		meta_t::get_key(key.m_key, data);
		m_keys.erase(key);
	}

	// get data id by index.
	virtual id_t get_id(IN const object_t& data) const
	{
		return meta_t::get_id(data);
	}

	// get index last object id.
	virtual id_t get_last() const
	{
		return (m_keys.empty()) ? id_t() : m_keys.rbegin()->m_id;
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename id_t, typename object_t, typename meta_t>
class Store
{
public:
	typedef std::shared_ptr<object_t> object_ptr;
	typedef Store<id_t, object_t, meta_t> super;
	std::map<id_t, object_ptr> m_datas;
	std::map<std::string, Index::ptr> m_indexs;

	// add index.
	template<typename key_t>
	inline void add_index(const std::string& name)
	{
		Index::ptr index(new Index<id_t, key_t, object_t, meta_t>());
		m_indexs[name] = index;
	}

	// remove index.
	inline void remove_index(const std::string& name)
	{
		m_indexs.erase(name);
	}

	// add object data in map.
	inline void add_data(IN const object_ptr& obj)
	{
		id_t id = meta_t::get_id(*obj);
		auto& old = m_datas[id];
		for (auto it = m_indexs.begin(); it != m_indexs.end(); ++it)
		{
			if (!old)
				(**it).setup(obj);
			else
				(**it).setup(old, obj);
		}
		old = obj;
	}

	// update object data in map.
	inline void update_data(IN const object_ptr& obj, IN int p, IN const char* v)
	{
		id_t id = meta_t::get_id(*obj);
		auto& old = m_datas[id];
		for (auto it = m_indexs.begin(); it != m_indexs.end(); ++it)
		{
			if ((**it).property_index() == p)
				(**it).setup(old, v);
		}
		old = obj;
	}

	// remove object data by object.
	inline void remove_data(IN const object_ptr& obj)
	{
		id_t id = meta_t::get_id(*obj);
		m_datas.erase(id);
		for (auto iti = m_indexs.begin(); iti != m_indexs.end(); ++iti)
		{
			(**iti).remove(obj);
		}
	}

	// remove object data by object id.
	inline void remove_data(IN const id_t& id)
	{
		auto it = m_datas.find(id);
		if (it == m_datas.end()) return;
		auto obj = it->second;
		m_datas.erase(it);
		for (auto iti = m_indexs.begin(); iti != m_indexs.end(); ++iti)
		{
			(**iti).remove(obj);
		}
	}
};



////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif