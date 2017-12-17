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

	void operator()(IN const Text& buf);

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
	uint32_t m_sync;
	SinkOption m_sink_option;
	SeverityLevel m_severity_level;
	uint32_t m_capacity;
	uint32_t m_max_sync_interval;

	// console sink.
	std::auto_ptr<FileSink> m_file_sink;
	std::auto_ptr<ConsoleSink> m_console_sink;

	// file sink option.
	std::string m_file_path;
	uint32_t m_file_roll_size;
	uint32_t m_flush_interval;
	OnChangedLogFile m_on_create;

	// status.
	bool m_running;

public:
	Impl();
	void init(IN Core& wrap) {}
};


////////////////////////////////////////////////////////////////////////////////
ECO_TYPE_IMPL(Core);
ECO_PROPERTY_STR_IMPL(Core, file_path);
ECO_PROPERTY_BOL_IMPL(Core, sync);
ECO_PROPERTY_VAL_IMPL(Core, SinkOption, sink_option);
ECO_PROPERTY_VAL_IMPL(Core, uint32_t, capacity);
ECO_PROPERTY_VAL_IMPL(Core, uint32_t, max_sync_interval);
ECO_PROPERTY_VAL_IMPL(Core, uint32_t, file_roll_size);
ECO_PROPERTY_VAL_IMPL(Core, uint32_t, flush_interval);
void Core::set_severity_level(IN const SeverityLevel& val)
{
	impl().m_severity_level = val;
}
Core& Core::severity_level(IN const SeverityLevel& val)
{
	impl().m_severity_level = val;
	return *this;
}
SeverityLevel& Core::severity_level()
{
	return impl().m_severity_level;
}
const SeverityLevel& Core::get_severity_level() const
{
	return impl().m_severity_level;
}


////////////////////////////////////////////////////////////////////////////////
Core::Impl::Impl()
	: m_sync(false)
	, m_severity_level(eco::log::info)
	, m_capacity(log_queue_size)
	, m_file_roll_size(log_file_roll_size)
	, m_file_path("./log/")
	, m_on_create(nullptr)
	, m_sink_option(eco::log::file_sink)
	, m_running(false)
	, m_flush_interval(0)
{}
void Handler::operator()(IN const Text& buf)
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

	if (!impl().m_sync)
	{
		impl().m_server.reset(new Server<Handler>);
		impl().m_server->set_message_handler(Handler(impl()));
		if (impl().m_capacity < log_min_queue_size)
			impl().m_capacity = log_min_queue_size;
		if (impl().m_max_sync_interval < log_min_sync_interval)
			impl().m_max_sync_interval = log_min_sync_interval;
		impl().m_server->run(1, "log");
		impl().m_server->set_capacity(impl().m_capacity);
		impl().m_server->set_max_sync_interval(impl().m_max_sync_interval);
	}

	// console logging output.
	if (eco::has(impl().m_sink_option, eco::log::console_sink))
	{
		impl().m_console_sink.reset(new ConsoleSink);
	}

	// file logging output.
	if (eco::has(impl().m_sink_option, eco::log::file_sink))
	{
		if (impl().m_file_roll_size < log_min_file_roll_size)
			impl().m_file_roll_size = log_min_file_roll_size;
		impl().m_file_sink.reset(new FileSink(
			impl().m_file_path,
			impl().m_file_roll_size,
			impl().m_flush_interval,
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
void Core::set_severity_level(IN const char* v)
{
	impl().m_severity_level = eco::log::Severity::get_level(v);
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
void Core::append(IN const Text& buf)
{
	if (impl().m_server != nullptr)
	{
		impl().m_server->post(buf);
	}
	else
	{
		Handler hdl(impl());
		hdl(buf);
	}
}


////////////////////////////////////////////////////////////////////////////////
Core& get_core()
{
	return eco::Singleton<Core>::instance();
}

////////////////////////////////////////////////////////////////////////////////
}}