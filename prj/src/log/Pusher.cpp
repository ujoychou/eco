#include "PrecHeader.h"
#include <eco/log/Pusher.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/DateTime.h>
#include <eco/thread/Thread.h>
#include <eco/filesystem/SourceFile.h>
#include <eco/log/Core.h>
#include <eco/Error.h>
#include <boost/algorithm/string.hpp>


namespace eco{;
namespace log{;
////////////////////////////////////////////////////////////////////////////////
const char* g_sev_name[] = 
{
	"trace",
	"debug", 
	"info",
	"warn",
	"error",
	"fatal",
	"none",
};
const char* g_sev_display[] = 
{
	"Trace",
	"Debug", 
	"Info ",
	"Warn ",
	"Error",
	"Fatal",
};

SeverityLevel Severity::get_level(IN const char* sev_name)
{
	for (size_t i=0; i<sizeof(g_sev_name)/sizeof(char*); ++i)
	{
		if (boost::iequals(g_sev_name[i], sev_name))
		{
			return static_cast<SeverityLevel>(i);
		}
	}
	return eco::log::info;	// default logging level.
}

const char* Severity::get_name(IN const SeverityLevel sev_level)
{
	if (sev_level > eco::log::none || sev_level < 0)
	{
		return "unknown";
	}
	return g_sev_name[sev_level];
}

const char* Severity::get_display(IN const SeverityLevel sev_level)
{
	if (sev_level >= eco::log::none || sev_level < 0)
	{
		return "Unknown";
	}
	return g_sev_display[sev_level];
}



////////////////////////////////////////////////////////////////////////////////
class Pusher::Impl
{
public:
	LogStream m_stream;
	SeverityLevel m_severity;
	std::auto_ptr<eco::filesystem::SourceFile> m_src;

	void init(IN Pusher& wrap)
	{
		m_severity = eco::log::none;
	}

	~Impl();

	void init(
		IN const char* file_name, 
		IN int file_line, 
		IN SeverityLevel sev_level,
		IN const char* domain);
};


////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(Pusher);
void Pusher::Impl::init(
	IN const char* file_name, 
	IN int file_line, 
	IN SeverityLevel sev_level,
	IN const char* domain)
{
	m_severity = sev_level;
	if (m_severity < get_core().get_severity_level())
	{
		return ;
	}

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
}
Pusher::Impl::~Impl()
{
	if (m_severity < get_core().get_severity_level())
	{
		return ;
	}

	// logging thread name.
	if (m_severity > eco::log::info)
	{
		m_stream << " #" << eco::this_thread::name();
	}

	// logging source file info.
	if (m_src.get() != nullptr)
	{
		m_stream << " #" << m_src->get_name()
			<< ":" << m_src->get_line();
	}

	// turn line '\n' must input into the string.
	m_stream.buffer().force_append('\n');
	get_core().append(m_stream.get_buffer());
}
////////////////////////////////////////////////////////////////////////////////
Pusher& Pusher::set(
	IN const char* file_name,
	IN int file_line, 
	IN SeverityLevel sev_level,
	IN const char* domain)
{
	impl().init(file_name, file_line, sev_level, domain);
	return *this;
}
LogStream& Pusher::stream()
{
	return impl().m_stream;
}


////////////////////////////////////////////////////////////////////////////////
}}

