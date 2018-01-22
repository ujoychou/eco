#ifndef ECO_REPOSITORY_H
#define ECO_REPOSITORY_H
/*******************************************************************************
@ name
data repository.

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
#include <eco/thread/CallOnce.h>
#include <unordered_map>
#include <map>



namespace eco{;


////////////////////////////////////////////////////////////////////////////////
template<
	typename identity_t, 
	typename value_t,
	typename container = std::unordered_map<identity_t, value_t >>
class Repository : public eco::Object<Repository<identity_t, value_t, container> >
{
public:
	typedef identity_t identity;
	typedef value_t value;
	typedef Repository<identity_t, value_t, container> repository;

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
	Repository()
	{}

	/*@ destructor. */
	virtual ~Repository()
	{}

	// repository size.
	inline size_t size() const
	{
		eco::Mutex::ScopeLock lock(m_data_map_mutex);
		return m_data_map.size();
	}

	/*@ add object to repository.*/
	inline void set(
		IN const identity& id,
		IN const value& val)
	{
		eco::Mutex::ScopeLock lock(m_data_map_mutex);
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

	/*@ get object from repository, create object derived from value object.
	*/
	template<typename object_t>
	inline value get_object(
		IN const identity& id,
		IN object_t*(*make_func)(IN const identity&)
		= &repository::make_object<object_t, identity>)
	{
		// find data in repository.
		eco::Mutex::ScopeLock lock(m_data_map_mutex);
		auto it = m_data_map.find(id);
		if (it != m_data_map.end())
		{
			return it->second;
		}
		// add val to repository.
		return (m_data_map[id] = value(make_func(id)));
	}

	/*@ get value from repository.
	*/
	inline value get(IN const identity& id)
	{
		// find data in repository.
		eco::Mutex::ScopeLock lock(m_data_map_mutex);
		auto it = m_data_map.find(id);
		if (it != m_data_map.end())
		{
			return it->second;
		}
		// add val to repository.
		return m_data_map[id];
	}

	/*@ get object from repository, if find failed throw exception.
	*/
	inline value at(IN const identity& id) const
	{
		// find data in repository.
		value val;
		if (!find(val, id))
		{
			EcoThrow(-1);
		}
		return val;
	}

	/*@ find object from repository.
	*/
	inline bool find(OUT value& val, IN  const identity& id) const
	{
		eco::Mutex::ScopeLock lock(m_data_map_mutex);
		typename container::const_iterator it = m_data_map.find(id);
		if (it == m_data_map.end())
		{
			return false;
		}
		val = it->second;
		return true;
	}

	/*@ find object from repository.
	*/
	template<typename function_t>
	inline bool find_if(OUT value& val, IN function_t func) const
	{
		eco::Mutex::ScopeLock lock(m_data_map_mutex);
		auto it = std::find_if(m_data_map.begin(), 
			m_data_map.end(), MapFinder<function_t>(func));
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
		eco::Mutex::ScopeLock lock(m_data_map_mutex);
		return (m_data_map.find(id) != m_data_map.end());
	}

	/*@ remove object from repository.*/
	inline value pop(IN const identity& id)
	{
		int eid = 0;
		value v = pop(id, eid);
		if (eid != 0)
		{
			EcoThrow(eid) << "repository pop fail.";
		}
		return v;
	}
	/*@ remove object from repository.*/
	inline value pop(IN const identity& id, OUT int& eid)
	{
		eco::Mutex::ScopeLock lock(m_data_map_mutex);
		value v;
		auto it = m_data_map.find(id);
		if (it == m_data_map.end())
		{
			eid = -1;
			return v;
		}

		v = it->second;
		m_data_map.erase(it);
		eid = 0;
		return v;
	}

	/*@ remove object from repository.*/
	inline void erase(IN const identity& id)
	{
		eco::Mutex::ScopeLock lock(m_data_map_mutex);
		auto it = m_data_map.find(id);
		if (it != m_data_map.end())
		{
			m_data_map.erase(it);
		}
	}

	/*@ clear object in this map.*/
	inline void clear()
	{
		eco::Mutex::ScopeLock lock(m_data_map_mutex);
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
		return m_data_map_mutex;
	}

////////////////////////////////////////////////////////////////////////////////
protected:
	// data map.
	container m_data_map;
	mutable eco::Mutex m_data_map_mutex;
};


////////////////////////////////////////////////////////////////////////////////
template<
	typename Identity,
	typename Value,
	typename Container = std::map < Identity, Value >>
class MapRepository : public Repository<Identity, Value, Container>
{};

////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif