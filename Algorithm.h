#ifndef ECO_ALGORITHM_H
#define ECO_ALGORITHM_H
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
template<typename set_t>
struct SetValueCompare
{
	typedef typename set_t::value_type value_t;
	typedef std::function<bool(const value_t& a, const value_t& b)> Func;
};
template<typename set_t>
inline void get_insert_set(
	OUT set_t& insert_set,
	IN  const set_t& old_set,
	IN  const set_t& new_set,
	IN  typename SetValueCompare<set_t>::Func equal)
{
	for (auto& new_it : new_set)
	{
		auto it_find = std::find_if(old_set.begin(), old_set.end(),
			[&](const typename set_t::value_type& old)->bool {
			return equal(old, new_it);
		});
		if (it_find == old_set.end())
		{
			insert_set.push_back(new_it);
		}
	}
}
template<typename set_t>
inline void get_compare_set(
	OUT set_t& insert_set,
	OUT set_t& delele_set,
	IN  const set_t& old_set,
	IN  const set_t& new_set,
	IN  typename SetValueCompare<set_t>::Func equal)
{
	get_insert_set(insert_set, old_set, new_set, equal);
	get_insert_set(delele_set, new_set, old_set, equal);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif