#ifndef ECO_THREAD_MAP_H
#define ECO_THREAD_MAP_H
/*******************************************************************************
@ name
data map.

@ function
it shared the data in the program, and it manage data's life cycle.


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Object.h>
#include <eco/thread/Mutex.h>
#include <unordered_map>
#include <map>



namespace eco{;


////////////////////////////////////////////////////////////////////////////////
template<
	typename identity_t, 
	typename value_t,
	typename container = std::unordered_map<identity_t, value_t >>
class MapT : public eco::Object<MapT<identity_t, value_t, container> >
{
public:
	typedef identity_t identity;
	typedef value_t value;

	template<typename value_iterator_find>
	class MapFinder
	{
	public:
		inline MapFinder(value_iterator_find& func) : m_func(func)
		{}

		inline bool operator()(typename container::value_type& val)
		{
			return m_func(val.second);
		}

		inline bool operator()(typename const container::value_type& val)
		{
			return m_func(val.second);
		}

	private:
		value_iterator_find& m_func;
	};

////////////////////////////////////////////////////////////////////////////////
public:
	/*@ constructor. */
	inline MapT() {}

	// map size.
	inline size_t size() const
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		return m_data_map.size();
	}

	/*@ add object to map.*/
	inline void set(
		IN const identity& id,
		IN const value& val)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		m_data_map[id] = val;
	}

	// make object.
	template<typename object_t, typename object_id>
	inline static object_t* make_object(IN const object_id& id)
	{
		object_t* obj = new object_t();
		obj->set_id(id);
		return obj;
	}

	/*@ get object from map, create object derived from value object.
	*/
	template<typename object_t>
	inline value get_object(
		IN const identity& id,
		IN object_t*(*make_func)(IN const identity&)
		= &make_object<object_t, identity>)
	{
		// find data in map.
		eco::Mutex::ScopeLock lock(m_mutex);
		auto it = m_data_map.find(id);
		if (it != m_data_map.end())
		{
			return it->second;
		}
		// add val to map.
		return (m_data_map[id] = value(make_func(id)));
	}

	/*@ get value from map.
	*/
	inline value get(IN const identity& id)
	{
		// find data in map.
		eco::Mutex::ScopeLock lock(m_mutex);
		auto it = m_data_map.find(id);
		if (it != m_data_map.end())
		{
			return it->second;
		}
		// add val to map.
		return m_data_map[id];
	}

	/*@ get object from map, if find failed throw exception.
	*/
	inline value at(IN const identity& id) const
	{
		// find data in map.
		value val;
		if (!find(val, id))
		{
			EcoThrow << "get object from eco map fail.";
		}
		return val;
	}

	/*@ find object from map.
	*/
	inline bool find(OUT value& val, IN  const identity& id) const
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		typename container::const_iterator it = m_data_map.find(id);
		if (it == m_data_map.end())
		{
			return false;
		}
		val = it->second;
		return true;
	}

	/*@ whether has the object.*/
	inline bool has(IN const identity& id) const
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		return (m_data_map.find(id) != m_data_map.end());
	}

	/*@ remove object from map.*/
	inline value pop(IN const identity& id)
	{
		int eid = 0;
		value v = pop(id, eid);
		if (eid != 0)
		{
			EcoThrow(eid) << "map pop fail.";
		}
		return v;
	}
	/*@ remove object from map.*/
	inline value pop(IN const identity& id, OUT int& eid)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		value v;
		auto it = m_data_map.find(id);
		if (it == m_data_map.end())
		{
			eid = eco::error;
			return v;
		}

		v = it->second;
		m_data_map.erase(it);
		eid = eco::ok;
		return v;
	}

	/*@ remove object from map.*/
	inline void erase(IN const identity& id)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		auto it = m_data_map.find(id);
		if (it != m_data_map.end())
		{
			m_data_map.erase(it);
		}
	}

	/*@ clear object in this map.*/
	inline void clear()
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		m_data_map.clear();
	}

	/*@ get raw data set.*/
	inline container& map()
	{
		return m_data_map;
	}

	/*@ get raw data set.*/
	inline const container& get_map() const
	{
		return m_data_map;
	}

	/*@ get raw data set.*/
	inline eco::Mutex& mutex() const
	{
		return m_mutex;
	}

////////////////////////////////////////////////////////////////////////////////
protected:
	// data map.
	container m_data_map;
	mutable eco::Mutex m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
template<typename identity_t, typename value_t>
class HashMap : public MapT<identity_t, value_t> {};

template<typename identity_t, typename value_t>
class Map : public MapT<identity_t, value_t, std::map<identity_t, value_t> > {};


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif