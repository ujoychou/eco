#include "PrecHeader.h"
#include <eco/log/Core.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/log/Pusher.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/filesystem/operations.hpp>
#include "FileSink.h"
#include "Server.h"


namespace eco{;
namespace log{;


//##############################################################################
//##############################################################################
////////////////////////////////////////////////////////////////////////////////
class ConsoleSink
{
public:
	inline void operator<<(IN const char* buf)
	{
		printf(buf);
	}
};


////////////////////////////////////////////////////////////////////////////////
class Handler
{
public:
	Handler() : m_core(nullptr)
	{}

	Handler(IN Core::Impl& imp) : m_core(&imp)
	{}

	// sync logging.
	void operator()(IN const eco::Bytes& buf, IN const SeverityLevel level);

	// async logging.
	void operator()(IN const Pack& buf);

private:
	Core::Impl* m_core;
};


////////////////////////////////////////////////////////////////////////////////
class Core::Impl
{
public:
	// async server.
	Server<Handler>::ptr m_server;

	// core option.
	uint32_t m_async;
	SinkOption m_sink_option;
	uint32_t m_capacity;
	uint32_t m_async_flush;

	// console sink.
	std::auto_ptr<FileSink> m_file_sink;
	std::auto_ptr<ConsoleSink> m_console_sink;
	SeverityLevel m_file_sev;
	SeverityLevel m_console_sev;

	// file sink option.
	std::string m_file_path;
	uint32_t m_file_roll_size;
	OnChangedLogFile m_on_create;

	// status.
	bool m_running;

public:
	Impl();
	void init(IN Core& wrap) {}
};


////////////////////////////////////////////////////////////////////////////////
//ECO_TYPE_IMPL(Core);
Core::Core()
{
	m_impl = new Impl;
	m_impl->init(*this);
}
Core::~Core()
{
	delete m_impl;
	m_impl = nullptr;
}
ECO_IMPL(Core)
ECO_PROPERTY_STR_IMPL(Core, file_path);
ECO_PROPERTY_BOL_IMPL(Core, async);
ECO_PROPERTY_VAV_IMPL(Core, SinkOption, sink_option);
ECO_PROPERTY_VAV_IMPL(Core, uint32_t, capacity);
ECO_PROPERTY_VAV_IMPL(Core, uint32_t, async_flush);
ECO_PROPERTY_VAV_IMPL(Core, uint32_t, file_roll_size);


////////////////////////////////////////////////////////////////////////////////
Core::Impl::Impl()
	: m_async(true)
	, m_capacity(queue_size)
	, m_file_roll_size(eco::log::file_roll_size)
	, m_file_path("./log/")
	, m_on_create(nullptr)
	, m_sink_option(eco::log::file_sink)
	, m_running(false)
	, m_file_sev(eco::log::info)
	, m_console_sev(eco::log::info)
{}
void Handler::operator()(IN const eco::Bytes& buf, IN const SeverityLevel level)
{
	if (m_core->m_file_sink.get() != nullptr && level >= m_core->m_file_sev)
	{
		(*m_core->m_file_sink).append(buf.c_str(), buf.size());
	}
	if (m_core->m_console_sink.get() != nullptr && level >= m_core->m_console_sev)
	{
		(*m_core->m_console_sink) << buf.c_str();
	}
}
void Handler::operator()(IN const Pack& buf)
{
	if (m_core->m_file_sink.get() != nullptr)
	{
		(*m_core->m_file_sink).append(buf.c_str(), buf.size());
	}
	if (m_core->m_console_sink.get() != nullptr)
	{
		(*m_core->m_console_sink) << buf.c_str();
	}
}

////////////////////////////////////////////////////////////////////////////////
void Core::run()
{
	// create file path, if file path is not exist.
	boost::system::error_code e;
	const std::string& path_v = impl().m_file_path;
	if (!boost::filesystem::exists(path_v, e))
	{
		if (!boost::filesystem::create_directories(path_v, e))
		{
			EcoThrow(e.value()) << e.message();
		}
	}

	if (impl().m_async)
	{
		impl().m_server.reset(new Server<Handler>);
		impl().m_server->set_message_handler(Handler(impl()));
		if (impl().m_capacity < min_queue_size)
			impl().m_capacity = min_queue_size;
		if (impl().m_async_flush < min_sync_interval)
			impl().m_async_flush = min_sync_interval;
		impl().m_server->run(1, "log");
		impl().m_server->set_capacity(impl().m_capacity);
		impl().m_server->set_sync_interval(impl().m_async_flush);
	}

	// console logging output.
	if (eco::has(impl().m_sink_option, eco::log::console_sink))
	{
		impl().m_console_sink.reset(new ConsoleSink);
	}

	// file logging output.
	if (eco::has(impl().m_sink_option, eco::log::file_sink))
	{
		if (impl().m_file_roll_size < min_file_roll_size)
			impl().m_file_roll_size = min_file_roll_size;

		// logging file flush is 0, use async flush interval instead.
		impl().m_file_sink.reset(new FileSink(
			impl().m_file_path,
			impl().m_file_roll_size, 0,
			false, impl().m_on_create));
	}
	m_impl->m_running = true;
}
void Core::stop()
{
	if (impl().m_server != nullptr)
	{
		impl().m_server->stop();
	}
}
void Core::join()
{
	if (impl().m_server != nullptr)
	{
		impl().m_server->join();
	}
}
bool Core::is_running() const
{
	return impl().m_running;
}
void Core::set_severity_level(IN const char* v, IN const int flag)
{
	set_severity_level(eco::log::Severity::get_level(v), flag);
}
void Core::set_severity_level(IN const SeverityLevel v, IN const int flag)
{
	if (flag == 0 || flag == 1)
		impl().m_file_sev = v;
	if (flag == 0 || flag == 2)
		impl().m_console_sev = v;
}
const SeverityLevel Core::get_severity_level() const
{
	return (impl().m_file_sev < impl().m_console_sev)
		? impl().m_file_sev : impl().m_console_sev;
}
void Core::add_file_sink(IN bool is_add)
{
	eco::set(impl().m_sink_option, eco::log::file_sink, is_add);
}
bool Core::has_file_sink() const
{
	return eco::has(impl().m_sink_option, eco::log::file_sink);
}
void Core::add_console_sink(IN bool is_add)
{
	eco::set(impl().m_sink_option, eco::log::console_sink, is_add);
}
bool Core::has_console_sink() const
{
	return eco::has(impl().m_sink_option, eco::log::console_sink);
}
void Core::set_file_on_create(IN eco::log::OnChangedLogFile& func)
{
	impl().m_on_create = func;
}
void Core::append(IN const eco::Bytes& buf, IN const SeverityLevel level)
{
	if (impl().m_server != nullptr)		// async
	{
		impl().m_server->post(buf);
	}
	else
	{
		Handler hdl(impl());			// sync
		hdl(buf, level);
	}
}


////////////////////////////////////////////////////////////////////////////////
Core& get_core()
{
	return eco::Singleton<Core>::instance();
}


////////////////////////////////////////////////////////////////////////////////
const char* const g_sev_name[] =
{
	"trace",
	"debug",
	"info",
	"warn",
	"error",
	"fatal",
	"none",
};
const char* const g_sev_display[] =
{
	"[trace]",
	"[debug]",
	"[info ]",
	"[warn ]",
	"[error]",
	"[fatal]",
};


////////////////////////////////////////////////////////////////////////////////
SeverityLevel Severity::get_level(IN const char* sev_name)
{
	for (size_t i = 0; i<sizeof(g_sev_name) / sizeof(char*); ++i)
	{
		if (strcmp(g_sev_name[i], sev_name) == 0)
		{
			return static_cast<SeverityLevel>(i);
		}
	}
	return eco::log::info;	// default logging level.
}


////////////////////////////////////////////////////////////////////////////////
const char* Severity::get_name(IN const SeverityLevel sev_level)
{
	if (sev_level > eco::log::none || sev_level < 0)
		return "unknown";
	return g_sev_name[sev_level];
}
const char* Severity::get_display(IN const SeverityLevel sev_level)
{
	if (sev_level >= eco::log::none || sev_level < 0)
		return "Unknown";
	return g_sev_display[sev_level];
}


////////////////////////////////////////////////////////////////////////////////
}}