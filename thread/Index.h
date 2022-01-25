#ifndef ECO_THREAD_INDEX_H
#define ECO_THREAD_INDEX_H
/*******************************************************************************
@ name
data array.

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
#include <vector>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<typename value_t>
class IndexArray : public eco::Object<IndexArray<value_t> >
{
public:
	typedef value_t value;

	/*@ constructor. */
	inline IndexArray() {}

	// array size.
	inline size_t size() const
	{
		std_lock_guard lock(m_mutex);
		return m_array.size();
	}

	/*@ add object to array.*/
	inline void push_back(
		IN const identity& id,
		IN const value& val)
	{
		std_lock_guard lock(m_mutex);
		m_array[id] = val;
	}

	/*@ find object from array.	*/
	template<typename function_t>
	inline bool find_if(OUT value& val, IN function_t& func) const
	{
		std_lock_guard lock(m_mutex);
		auto it = std::find_if(m_array.begin(), m_array.end(), func);
		if (it == m_array.end())
		{
			return false;
		}
		val = it->second;
		return true;
	}

	/*@ remove object from array.*/
	inline value pop_back()
	{
		value v = std::move(m_array.back());
		m_array.pop_back();
		return move(v);
	}

	/*@ clear object in this array.*/
	inline void clear()
	{
		std_lock_guard lock(m_mutex);
		m_array.clear();
	}

	/*@ get raw data array.*/
	inline std::vector<value_t>& array()
	{
		return m_array;
	}

	/*@ get raw data array.*/
	inline const std::vector<value_t>& get_array() const
	{
		return m_array;
	}

	/*@ get raw data array.*/
	inline std_mutex& mutex() const
	{
		return m_mutex;
	}

////////////////////////////////////////////////////////////////////////////////
protected:
	std::vector<value_t>	m_array;
	mutable std_mutex		m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif