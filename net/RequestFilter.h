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
	filter_none			= 0x0000,
	filter_sess_authed  = 0x0001,
	filter_conn_authed  = 0x0002,
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

	inline bool sess_authed() const
	{
		return eco::has(m_value, filter_sess_authed);
	}
	inline void set_sess_authed(IN bool is)
	{
		eco::set(m_value, filter_sess_authed, is);
	}

	inline bool conn_authed() const
	{
		return eco::has(m_value, filter_conn_authed);
	}
	inline void set_conn_authed(IN bool is)
	{
		eco::set(m_value, filter_conn_authed, is);
	}

private:
	RequestFilterValue m_value;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif