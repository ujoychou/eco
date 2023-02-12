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
#include <eco/std/mutex.h>
#include <unordered_set>
#include <set>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<
	typename identity_t, 
	typename std_set = std::unordered_set<identity_t>>
class SetT : public eco::Object<SetT<identity_t, std_set> >
{
public:
	typedef identity_t identity;

	/*@ constructor. */
	inline SetT() {}

	// map size.
	inline size_t size() const
	{
		std_lock_guard lock(m_mutex);
		return m_data_set.size();
	}

	/*@ add object to map.*/
	inline void add(IN const identity& id)
	{
		std_lock_guard lock(m_mutex);
		m_data_set.insert(id);
	}

	/*@ whether has the object.*/
	inline bool has(IN const identity& id) const
	{
		std_lock_guard lock(m_mutex);
		return (m_data_set.find(id) != m_data_set.end());
	}

	/*@ remove object from map.*/
	inline void erase(IN const identity& id)
	{
		std_lock_guard lock(m_mutex);
		auto it = m_data_set.find(id);
		if (it != m_data_set.end())
		{
			m_data_set.erase(it);
		}
	}

	/*@ clear object in this map.*/
	inline void clear()
	{
		std_lock_guard lock(m_mutex);
		m_data_set.clear();
	}

	/*@ get raw data set.*/
	inline std_set& set() { return m_data_set; }

	/*@ get raw data set.*/
	inline const std_set& get_set() const { return m_data_set; }

	/*@ get raw data set.*/
	inline std_mutex& mutex() const { return m_mutex; }

protected:
	// data map.
	std_set m_data_set;
	mutable std_mutex m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
template<typename identity_t>
class HashSet : public SetT<identity_t> {};
template<typename identity_t>
class Set : public SetT<identity_t, std::set<identity_t>> {};
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif