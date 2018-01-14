#ifndef ECO_PERSIST_H
#define ECO_PERSIST_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/meta/Meta.h>


////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(persist);


////////////////////////////////////////////////////////////////////////////////
// database upgrade solution.
class Version
{
public:
	uint32_t	m_value;
	std::string m_module;
	std::string m_update_time;
	eco::meta::Timestamp m_timestamp;

public:
	inline operator const uint32_t() const
	{
		return m_value;
	}

	inline Version& value(IN const uint32_t v, IN bool new_record = false)
	{
		m_value = v;
		if (!new_record)
			m_timestamp.update();
		else
			m_timestamp.set_value(eco::meta::inserted);
		return *this;
	}
};


////////////////////////////////////////////////////////////////////////////////
class VersionMeta : public eco::Meta<Version>
{
public:

};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(persist);
ECO_NS_END(eco);
#endif