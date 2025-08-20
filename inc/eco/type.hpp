#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-08-21] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/prec.hpp>
#include <vector>
#include <algorithm>


eco_namespace(eco)
////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename compare_t>
struct vector_map
{
	typedef typename std::vector<type_t>::iterator iterator;
	typedef typename std::vector<type_t>::const_iterator const_iterator;

	template<typename type_view_t>
    inline type_t* set(const type_view_t& value)
    {
        auto it = lower_bound<type_view_t>(value);
        if (it == m_data.end())
			it = m_data.emplace(it, value);
		else if (m_compare(*it, value) == 0)
			*it = value;
        return &(*it);
    }
    
	template<typename type_view_t>
    inline type_t* find(const type_view_t& value)
    {
        auto it = lower_bound<type_view_t>(value);
        return (it != m_data.end() && *it == value) ? &(*it) : nullptr;
    }

	template<typename type_view_t>
	inline iterator lower_bound(const type_view_t& value)
	{
		return std::lower_bound(m_data.begin(), m_data.end(), value, m_compare);
	}

    template<typename type_view_t>
	inline const_iterator lower_bound(const type_view_t& value) const
	{
		return std::lower_bound(m_data.begin(), m_data.end(), value, m_compare);
	}

private:
    compare_t m_compare;    
    std::vector<type_t> m_data;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco)