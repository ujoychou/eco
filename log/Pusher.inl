#ifndef ECO_LOG_PUSHER_INL
#define ECO_LOG_PUSHER_INL
////////////////////////////////////////////////////////////////////////////////
#include <eco/log/Core.h>
#include <eco/DateTime.h>
#include <eco/thread/Thread.h>


namespace eco{;
namespace log{;


////////////////////////////////////////////////////////////////////////////////
template<typename LogStream>
inline PusherT<LogStream>::PusherT()
	: m_severity(none)
	, m_file_line(0)
	, m_file_name(nullptr)
{}


////////////////////////////////////////////////////////////////////////////////
template<typename LogStream>
PusherT<LogStream>::~PusherT()
{
	if (m_severity < get_core().get_severity_level())
	{
		return ;
	}

	// logging thread name.
	const char* t_name = eco::this_thread::name();
	if (m_severity > eco::log::info && !eco::empty(t_name))
	{
		m_stream << " &" << t_name;
	}
	// logging source file info.
	if (m_file_name != nullptr)
	{
		m_stream << " @" << m_file_name << '.' << m_file_line;
	}
	m_stream << eco::end();
	
	// turn line '\n' must input into the string.
	m_stream.buffer().force_append('\n');
	get_core().append(m_stream.get_bytes(), m_severity);
}


////////////////////////////////////////////////////////////////////////////////
template<typename LogStream>
PusherT<LogStream>& PusherT<LogStream>::set(
	IN const char* file_name, IN int file_line, IN SeverityLevel sev_level)
{
	assert(sev_level >= get_core().get_severity_level());
	m_severity = sev_level;
	
	// info< logging no need to save source file info.
	if (sev_level > eco::log::info)
	{
		uint32_t pos = -1;
		m_file_name = (
			(pos = find_last(file_name, '/')) != -1 ||
			(pos = find_last(file_name, '\\')) != -1)
			? &file_name[pos + 1] : file_name;
		m_file_line = file_line;
	}

	// default domain is empty string.
	eco::date_time::Timestamp now(eco::date_time::fmt_std_m);
	m_stream << now.get_value() <= eco::this_thread::id_string()
		<= Severity::get_display(sev_level) < ' ';
	return *this;
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif