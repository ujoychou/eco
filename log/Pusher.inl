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
PusherT<LogStream>::~PusherT()
{
	if (m_severity < get_core().get_severity_level())
	{
		return ;
	}

	// logging thread name.
	const char* t_name = eco::this_thread::name();
	if (m_severity > eco::log::info && t_name[0] != 0)
	{
		m_stream << " #" << t_name;
	}

	// logging source file info.
	if (m_src.get() != nullptr)
	{
		m_stream << " #" << m_src->get_name()
			<< ":" << m_src->get_line();
	}
	m_stream << '$' << m_stream.size();

	// turn line '\n' must input into the string.
	m_stream.buffer().force_append('\n');
	get_core().append(m_stream.get_bytes());
}


////////////////////////////////////////////////////////////////////////////////
template<typename LogStream>
PusherT<LogStream>& PusherT<LogStream>::set(
	IN const char* file_name,
	IN int file_line, 
	IN SeverityLevel sev_level,
	IN const char* domain)
{
	assert(sev_level >= get_core().get_severity_level());
	m_severity = sev_level;
	
	// info< logging no need to save source file info.
	if (sev_level > eco::log::info)
	{
		m_src.reset(
			new eco::filesystem::SourceFile(file_name, file_line));
	}

	// default domain is empty string.
	eco::date_time::Timestamp now(eco::date_time::fmt_std_m);
	if (domain == nullptr || domain[0] == 0)
	{
		m_stream << now.get_value()
			<< " " << eco::this_thread::id_string()
			<< " " << Severity::get_display(sev_level) << ": ";
	}
	else
	{
		m_stream << now.get_value()
			<< " " << eco::this_thread::id_string()
			<< " " << Severity::get_display(sev_level)
			<< ": " << domain << ": ";
	}
	return *this;
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif