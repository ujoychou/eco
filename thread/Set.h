#ifndef ECO_THREAD_SET_H
#define ECO_THREAD_SET_H
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
	typename container = std::unordered_set<identity_t>>
class SetT : public eco::Object<SetT<identity_t, container> >
{
public:
	typedef identity_t identity;

	/*@ constructor. */
	inline SetT() {}

	// map size.
	inline size_t size() const
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		return m_data_map.size();
	}

	/*@ add object to map.*/
	inline void add(IN const identity& id)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		m_data_map.insert(id);
	}

	/*@ whether has the object.*/
	inline bool has(IN const identity& id) const
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		return (m_data_map.find(id) != m_data_map.end());
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
	inline container& set()
	{
		return m_data_map;
	}

	/*@ get raw data set.*/
	inline const container& get_set() const
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
template<typename identity_t>
class HashSet : public SetT<identity_t> {};

template<typename identity_t>
class Set : public SetT<identity_t, std::set<identity_t>> {};


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif