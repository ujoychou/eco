#ifndef ECO_NET_REQUEST_FILTER_H
#define ECO_NET_REQUEST_FILTER_H
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-17.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Project.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
enum
{
	filter_none				= 0x00000000,
	filter_closed_session	= 0x00000001,
	filter_opened_session	= 0x00000002,
};
typedef uint32_t RequestFilterValue;


////////////////////////////////////////////////////////////////////////////////
class RequestFilter
{
public:
	explicit RequestFilter(IN uint32_t v = filter_none) : m_value(v)
	{}

	inline bool has_filter() const
	{
		return m_value != filter_none;
	}

	inline bool closed_session() const
	{
		return eco::has(m_value, filter_closed_session);
	}
	inline void set_closed_session(IN bool is)
	{
		eco::set(m_value, filter_closed_session, is);
	}

	inline bool opened_session() const
	{
		return eco::has(m_value, filter_opened_session);
	}
	inline void set_opened_session(IN bool is)
	{
		eco::set(m_value, filter_opened_session, is);
	}

private:
	RequestFilterValue m_value;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif