#include "Pch.h"
#include <eco/log/Core.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/rx/RxImpl.h>
#include <eco/log/Pusher.h>
#include <eco/log/Type.h>
#include <boost/filesystem/operations.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "Server.h"
#include "FileSink.h"


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(this_thread);
////////////////////////////////////////////////////////////////////////////////
// thread local log buf.
thread_local eco::log::PusherT<eco::String>* t_logbuf = 0;
eco::log::PusherT<eco::String>& logbuf()
{
	if (t_logbuf == 0) 
	{
		t_logbuf = new eco::log::PusherT<eco::String>();
	}
	t_logbuf->stream().clear();
	return *t_logbuf;
}
ECO_NS_END(this_thread);


//##############################################################################
//##############################################################################
ECO_NS_BEGIN(log);
////////////////////////////////////////////////////////////////////////////////
class ConsoleSink
{
public:
	inline void operator<<(IN const char* buf)
	{
		std::cout << buf;
	}
};


////////////////////////////////////////////////////////////////////////////////
class Handler
{
public:
	inline Handler() : m_core(nullptr)
	{}

	inline Handler(IN Core::Impl& imp) : m_core(&imp)
	{}

	// sync logging.
	inline void operator()(
		IN const eco::Bytes& buf,
		IN SeverityLevel level);
	inline void cout(
		IN const eco::Bytes& buf,
		IN SeverityLevel level);

	// async logging.
	inline void operator()(IN const Pack& buf);
	// nouse: compatible with eco::Worker<>.
	inline void operator()(IN const eco::Bytes& buf) {}

private:
	Core::Impl* m_core;
};


////////////////////////////////////////////////////////////////////////////////
class Core::Impl
{
	ECO_IMPL_INIT(Core);
public:
	Impl();

	// async server.
	Server<eco::log::Handler>::ptr m_server;

	// core option.
	uint16_t m_init;
	uint16_t m_async;
	SinkOption m_sink_option;
	uint32_t m_capacity;
	uint32_t m_async_flush;			// async flush millseconds.

	// console sink.
	std_mutex m_sync_mutex;
	SeverityLevel m_file_sev;
	SeverityLevel m_console_sev;
	std::unique_ptr<FileSink> m_file_sink;
	std::unique_ptr<ConsoleSink> m_console_sink;
	
	// file sink option.
	uint32_t m_file_roll_size;
	std::string m_file_path;
	OnChangedLogFile m_on_create;
};


////////////////////////////////////////////////////////////////////////////////
ECO_SINGLETON_IMPL(Core);
ECO_PROPERTY_STR_IMPL(Core, file_path);
ECO_PROPERTY_BOL_IMPL(Core, async);
ECO_PROPERTY_VAR_IMPL(Core, SinkOption, sink_option);
ECO_PROPERTY_VAR_IMPL(Core, uint32_t, capacity);
ECO_PROPERTY_VAR_IMPL(Core, uint32_t, async_flush);
ECO_PROPERTY_VAR_IMPL(Core, uint32_t, file_roll_size);
Core& Core::get() { return eco::Singleton<Core>::get(); }


////////////////////////////////////////////////////////////////////////////////
Core::Impl::Impl()
	: m_init(false)
	, m_async(true)
	, m_sink_option(eco::log::console_sink | eco::log::file_sink)
	, m_capacity(queue_size)
	, m_async_flush(3000)
	, m_file_sev(eco::log::debug)
	, m_console_sev(eco::log::info)
	, m_file_roll_size(eco::log::file_roll_size)
	, m_file_path("log")
	, m_on_create(nullptr)
{}


////////////////////////////////////////////////////////////////////////////////
void Handler::cout(const eco::Bytes& buf, SeverityLevel level)
{
	if (m_core->m_console_sink.get() && level >= m_core->m_console_sev)
	{
		(*m_core->m_console_sink) << buf.c_str();
	}
}
void Handler::operator()(IN const eco::Bytes& buf, IN SeverityLevel level)
{
	if (m_core->m_file_sink.get() && level >= m_core->m_file_sev)
	{
		(*m_core->m_file_sink).append(buf.c_str(), buf.size());
	}
	Handler::cout(buf, level);
}


////////////////////////////////////////////////////////////////////////////////
void Handler::operator()(IN const Pack& buf)
{
	if (m_core->m_file_sink.get())
	{
		(*m_core->m_file_sink).append(buf.c_str(), buf.size());
	}
}

////////////////////////////////////////////////////////////////////////////////
void Core::run()
{
	if (impl().m_init) { return; }

	// create file path, if file path is not exist.
	boost::system::error_code e;
	const std::string& path_v = impl().m_file_path;
	if (!boost::filesystem::exists(path_v, e))
	{
		if (!boost::filesystem::create_directories(path_v, e))
		{
			ECO_THROW(e.value()) << e.message();
		}
	}

	if (impl().m_async)
	{
		if (impl().m_capacity < min_queue_size)
			impl().m_capacity = min_queue_size;
		if (impl().m_async_flush < min_sync_interval)
			impl().m_async_flush = min_sync_interval;
		impl().m_server.reset(new Server<eco::log::Handler>);
		impl().m_server->set_handler(eco::log::Handler(impl()));
		impl().m_server->set_capacity(impl().m_capacity);
		impl().m_server->set_sync_interval(impl().m_async_flush);
		impl().m_server->run("eco_log");
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
	impl().m_init = true;
}


////////////////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////////////////
void Core::set_severity_level(IN const char* v, IN int flag)
{
	set_severity_level(eco::log::Severity::get_level(v), flag);
}
void Core::set_severity_level(IN SeverityLevel v, IN int flag)
{
	if (flag == 0 || flag == 1)
		impl().m_file_sev = v;
	if (flag == 0 || flag == 2)
		impl().m_console_sev = v;
}
SeverityLevel Core::severity_level() const
{
	return (impl().m_file_sev < impl().m_console_sev)
		? impl().m_file_sev : impl().m_console_sev;
}


////////////////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////////////////
void Core::set_file_on_create(IN eco::log::OnChangedLogFile& func)
{
	impl().m_on_create = func;
}
void Core::append(IN const eco::Bytes& buf, IN SeverityLevel level)
{
	if (impl().m_server == nullptr)
	{
		std_lock_guard lock(impl().m_sync_mutex);
		Handler(impl()).operator()(buf, level);
		return;
	}
	
	try
	{
		// async write file.
		if (impl().m_file_sink.get() && level >= impl().m_file_sev)
		{
			impl().m_server->channel().post(buf);
		}
		Handler(impl()).cout(buf, level);
	}
	catch (std::exception& e)
	{
		// buffer size is larger than logging pack.
		eco::cout() << e.what();
		impl().m_server->post(eco::Bytes(e.what()));
		impl().m_server->post(eco::Bytes(buf.c_str(), 1000));
	}
}


////////////////////////////////////////////////////////////////////////////////
const char* const g_sev_name[] =
{
	"trace",
	"debug",
	"info",
	"key",
	"warn",
	"error",
	"fatal",
};
const char* const g_sev_display[] =
{
	"[trace]",
	"[debug]",
	"[info ]",
	"[key  ]",
	"[warn ]",
	"[error]",
	"[fatal]",
};
SeverityLevel Severity::get_level(IN const char* sev_name)
{
	for (size_t i = 0; i < sizeof(g_sev_name) / sizeof(char*); ++i)
	{
		if (strcmp(g_sev_name[i], sev_name) == 0)
		{
			return static_cast<SeverityLevel>(i + eco::log::trace);
		}
	}
	return eco::log::info;	// default logging level.
}


////////////////////////////////////////////////////////////////////////////////
const char* Severity::get_name(IN SeverityLevel sev_level)
{
	if (sev_level > eco::log::fatal || sev_level < eco::log::trace)
		return "unknown";
	return g_sev_name[sev_level - eco::log::trace];
}
const char* Severity::get_display(IN SeverityLevel sev_level)
{
	if (sev_level > eco::log::fatal || sev_level < eco::log::trace)
		return "[unknown]";
	return g_sev_display[sev_level - eco::log::trace];
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(log);
ECO_NS_END(eco);