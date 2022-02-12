#include "Pch.h"
#include <eco/App.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/rx/RxApp.h>
#include <eco/sys/Sys.h>
#include <eco/cmd/Engine.h>
#include <eco/net/TcpServer.h>
#ifdef WIN32
#	include <eco/sys/WinDump.h>
#	include <eco/sys/WinConsoleEvent.h>
#endif
#include <eco/thread/Monitor.h>
#include <eco/filesystem/Path.h>
#include <eco/service/dev/Cluster.h>
#include <boost/filesystem/operations.hpp>
#include <vector>
#include "Eco.ipp"


ECO_NS_BEGIN(eco);
extern void create_eco();
////////////////////////////////////////////////////////////////////////////////
class App::Impl
{
public:
	static App* s_app;
	static std_thread::id s_app_thread_id;
	static eco::Monitor s_monitor;
	static std_atomic_uint32_t s_app_counts;
	static std::vector<std::string> s_params;
	static std::string s_exe_path;
	static std::string s_exe_file;
	static std::string s_init_path;

public:
	App* m_app;
	std::string m_name;
	std::string m_module_file;
	std::string m_module_path;
	eco::Config m_config;
	std::string m_config_file;
	mutable eco::atomic::State m_state;
	std::vector<RxDll::ptr> m_erx_set;
	std::vector<eco::Persist> m_persist_set;
	net::TcpServer m_provider;
	std::vector<net::TcpClient> m_consumer_set;
	std::vector<net::AddressSet> m_router_set;
	eco::loc::Locale m_locale;
	
public:
	inline Impl() : m_app(0)
	{}

	inline ~Impl()
	{}

	inline void init(IN App& parent)
	{
		m_app = &parent;
	}

	inline void set_name(IN const char* v)
	{
		m_name = v;
		if (eco::empty(eco::cmd::engine().home().name()))
			eco::cmd::engine().home().name(m_name.c_str());
	}

	inline void set_provider_service(IN const char* v)
	{
		m_provider.get_option().set_name(v);
		if (m_name.empty()) set_name(v);
	}

	inline bool ready() const
	{
		for (auto it = m_persist_set.begin(); it != m_persist_set.end(); ++it)
		{
			const eco::Persist& p = *it;
			if (!p.ready()) return false;
		}
		for (auto it = m_consumer_set.begin(); it != m_consumer_set.end(); ++it)
		{
			const eco::net::TcpClient& c = *it;
			if (!c.ready()) return false;
		}
		return true;
	}

	// wait master finish init app.
	inline void wait_master(App& app);
	// 1.wait slaves exit and master exit.
	// 2.last app exit clear log/eco/provider, this will be more convenient.
	inline void wait_slaves(App& app);

	// is app a master.
	inline bool master(const App& app) const
	{
		return s_app == &app;
	}

	// wait command and provider thread.
	void init(IN App* app, IN void* module_func_addr);
	void load(IN App& app);
	// exit app and clear provider and consumer.
	void exit(IN App& app, IN bool error);

public:
	// log eco cmd
	inline void init_log();
	inline void init_eco();

	// locale multi language.
	inline void init_locale();
	inline std::string get_locale_path(
		const std::string& path,
		const eco::StringAny& any);

	// erx dll
	inline bool enable_erx() const;
	inline void on_rx_init();
	inline void on_rx_cmd();
	inline void on_rx_load();
	inline void on_rx_exit();
	inline RxDll::ptr get_erx(IN const char*);

	// service
	inline void init_router();
	inline void init_consumer();
	inline void start_consumer();
	inline void init_provider();
	inline void start_provider();
	inline net::TcpClient add_consumer(IN eco::net::Address& addr);
	inline net::TcpClient add_consumer(IN eco::net::AddressSet& addr);
	inline net::AddressSet init_address(ContextNode& node);
	inline void init_option(net::TcpOption& option, ContextNode& node);

	// persist
	inline void init_persist();
	inline void start_persist();
	inline eco::Persist add_persist(eco::persist::Address& addr);
};
////////////////////////////////////////////////////////////////////////////////
App*						App::Impl::s_app = 0;
std_thread::id				App::Impl::s_app_thread_id;
eco::Monitor				App::Impl::s_monitor;
std_atomic_uint32_t			App::Impl::s_app_counts;
std::vector<std::string>	App::Impl::s_params;
std::string					App::Impl::s_init_path;
std::string					App::Impl::s_exe_path;
std::string					App::Impl::s_exe_file;


//##############################################################################
//##############################################################################
////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_log()
{
	// #.read logging config.
	eco::StringAny v;
	if (m_config.find(v, "logging/async"))
		eco::log::Core::get().set_async(v);
	if (m_config.find(v, "logging/level"))
		eco::log::Core::get().set_severity_level(v.c_str());
	if (m_config.find(v, "logging/memcache"))
		eco::log::Core::get().set_capacity(int(double(v) * 1024 * 1024));
	if (m_config.find(v, "logging/async_flush"))
		eco::log::Core::get().set_async_flush(v);
	if (m_config.has("logging/file_sink"))
	{
		eco::log::Core::get().add_file_sink(true);
		if (m_config.find(v, "logging/file_sink/level"))
			eco::log::Core::get().set_severity_level(v.c_str(), 1);
		if (m_config.find(v, "logging/file_sink/file_path"))
			eco::log::Core::get().set_file_path(v.c_str());
		if (m_config.find(v, "logging/file_sink/roll_size"))
			eco::log::Core::get().set_file_roll_size(
				uint32_t(double(v) * 1024 * 1024));

		// set logging file path.
		std::string file(m_module_path);
		file += eco::log::Core::get().file_path();
		eco::filesystem::add_path_suffix(file);
		eco::log::Core::get().set_file_path(file.c_str());
	}
	else
	{
		eco::log::Core::get().add_file_sink(false);
	}

	if (m_config.has("logging/console_sink"))
	{
		eco::log::Core::get().add_console_sink(true);
		if (m_config.find(v, "logging/console_sink/level"))
			eco::log::Core::get().set_severity_level(v.c_str(), 2);
	}
	else
	{
		eco::log::Core::get().add_console_sink(false);
	}
	eco::log::Core::get().run();
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_eco()
{
	StringAny v;
	if (m_config.find(v, "eco/task_thread_size"))
		Eco::Impl::set_task_thread_size(v);
	Eco::get().impl().start();
}


////////////////////////////////////////////////////////////////////////////////
inline std::string App::Impl::get_locale_path(
	const std::string& path, const eco::StringAny& any)
{
	std::string f = m_module_path;
	f += path;
	f += '/';
	f += any.c_str();
	return f;
}
inline void App::Impl::init_locale()
{
	// locale setting.
	eco::ContextNode node = m_config.find_node("locale");
	if (node.null() || !node.has_children()) return;
	const StringAny* any = node.find("default");
	if (any) m_locale.set_default_(any->c_str());

	// language setting.
	eco::ContextNodeSet node_set = node.get_children();
	for (auto it = node_set.begin(); it != node_set.end(); ++it)
	{
		const ContextNode& lang = *it;
		std::string path = lang.get("path");
		std::string ver = lang.get("version");
		m_locale.set_language_info(lang.name(), path.c_str(), ver.c_str());

		eco::ContextNodeSet file_set = lang.children();
		if (file_set.null() || file_set.empty()) continue;
		for (auto itf = file_set.begin(); itf != file_set.end(); ++itf)
		{
			const eco::ContextNode& file = *itf;
			if (eco::equal(file.name(), "word"))
			{
				std::string f = get_locale_path(path, file.get("file"));
				eco::StringAny m = file.get("module");
				m_locale.add_word_file(lang.name(), f.c_str(), m.c_str());
			}
			else if (eco::equal(file.name(), "error"))
			{
				std::string f = get_locale_path(path, file.get("file"));
				eco::StringAny m = file.get("module");
				m_locale.add_error_file(lang.name(), f.c_str(), m.c_str());
			}
		}// end for.
	}// end for.
}


////////////////////////////////////////////////////////////////////////////////
inline bool App::Impl::enable_erx() const
{
	return m_config.has("erx");
}
inline void App::Impl::on_rx_init()
{
	if (enable_erx())
	{
		// load erx config in config file.
		eco::Context rx_set;
		m_config.get_property_set(rx_set, "erx");

		// load erx dll
		for (auto it = rx_set.begin(); it != rx_set.end(); ++it)
		{
			RxDll::ptr rx(new RxDll(it->value(), it->name()));
			m_erx_set.push_back(rx);
		}

		// notify erx to init.
		for (auto it = m_erx_set.begin(); it != m_erx_set.end(); ++it)
		{
			(**it).on_init(*m_app);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::on_rx_cmd()
{
	if (enable_erx())
	{
		// notify erx to init command.
		for (auto it = m_erx_set.begin(); it != m_erx_set.end(); ++it)
		{
			(**it).on_cmd();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::on_rx_load()
{
	if (enable_erx())
	{
		// notify erx to init command.
		for (auto it = m_erx_set.begin(); it != m_erx_set.end(); ++it)
		{
			(**it).on_load();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::on_rx_exit()
{
	try 
	{
		if (enable_erx())
		{
			// notify erx to exit.
			for (auto it = m_erx_set.begin(); it != m_erx_set.end(); ++it)
			{
				if ((**it).message() >= rx_msg_init)
				{
					(**it).on_exit();
				}
			}
			// unload erx.
			m_erx_set.clear();
		}
	}
	catch (std::exception& e)
	{
		ECO_ERROR << e.what();
	}
}


////////////////////////////////////////////////////////////////////////////////
RxDll::ptr App::Impl::get_erx(IN const char* name_)
{
	for (auto it = m_erx_set.begin(); it != m_erx_set.end(); ++it)
	{
		if (eco::equal((**it).name(), name_))
		{
			return *it;
		}
	}
	return nullptr;
}
RxDll::ptr App::get_erx(IN const char* name_)
{
	return impl().get_erx(name_);
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_router()
{
	// #.init router.
	eco::ContextNodeSet nodes = m_config.find_children("router");
	if (nodes.null() || nodes.size() == 0) return;

	// #.init router.
	for (auto r = nodes.begin(); r != nodes.end(); ++r)
	{
		// router/front_router/
		eco::net::AddressSet addr_set;
		addr_set.set_name(r->name());
		// router/front_router/sh_dx.
		auto& prop_set = r->get_property_set();
		for (auto it = prop_set.begin(); it != prop_set.end(); ++it)
		{
			addr_set.add().name(it->name()).set(it->get_value());
		}
		m_router_set.push_back(addr_set);
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_option(net::TcpOption& option, ContextNode& node)
{
	const eco::StringAny* v = nullptr;
	if (nullptr != (v = node.find("router")))
		option.set_router(v->c_str());
	if (nullptr != (v = node.find("no_delay")))
		option.set_no_delay(*v);
	if (nullptr != (v = node.find("websocket")))
		option.set_websocket(*v);
	if (nullptr != (v = node.find("heartbeat_rhythm")))
		option.set_heartbeat_rhythm(*v);
	if (nullptr != (v = node.find("heartbeat_recv_sec")))
		option.set_heartbeat_recv_sec(*v);
	if (nullptr != (v = node.find("heartbeat_send_sec")))
		option.set_heartbeat_send_sec(*v);
	if (nullptr != (v = node.find("context_capacity")))
		option.set_context_capacity(*v);
	if (nullptr != (v = node.find("send_capacity")))
		option.set_send_capacity(*v);
	if (nullptr != (v = node.find("send_buffer_size")))
		option.set_send_buffer_size(*v);
	if (nullptr != (v = node.find("recv_buffer_size")))
		option.set_recv_buffer_size(*v);
	if (nullptr != (v = node.find("send_low_watermark")))
		option.set_send_low_watermark(*v);
	if (nullptr != (v = node.find("recv_low_watermark")))
		option.set_recv_low_watermark(*v);
	if (nullptr != (v = node.find("max_byte_size")))
		option.set_max_byte_size(int(double(*v) * 1024 * 1024));
}


////////////////////////////////////////////////////////////////////////////////
inline eco::net::AddressSet App::Impl::init_address(eco::ContextNode& node)
{
	const eco::ContextNode* child = nullptr;
	if (node.has_children())
	{
		child = node.get_children().find("address");
		if (child == nullptr)
		{
			ECO_THROW(node.name()) < " has no address config.";
		}
	}

	eco::net::AddressSet addr_set;
	const StringAny* v = child->find("router");
	if (v != nullptr)
	{
		auto it = std::find_if(m_router_set.begin(), m_router_set.end(),
			[&v](IN const net::AddressSet& a) -> bool {
			return eco::equal(a.name(), v->c_str());
		});
		if (it == m_router_set.end())
		{
			ECO_THROW(it->name()) < " has no router " < v->c_str();
		}
		addr_set.add_copy(*it);
		addr_set.set_mode(eco::net::router_mode);
	}
	else
	{
		auto& props = child->property_set();
		for (auto it = props.begin(); it != props.end(); ++it)
		{
			addr_set.add().name(it->name()).set(it->value());
		}
		addr_set.set_mode(eco::net::service_mode);
	}
	addr_set.set_name(node.name());
	return addr_set;
}
net::TcpClient App::Impl::add_consumer(IN eco::net::Address& addr)
{
	eco::net::TcpClient client;
	client.add_address(addr);
	m_consumer_set.push_back(client);
	return m_consumer_set.back();
}
net::TcpClient App::Impl::add_consumer(IN eco::net::AddressSet& addr)
{
	eco::net::TcpClient client;
	client.set_address(addr);
	m_consumer_set.push_back(client);
	return m_consumer_set.back();
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_consumer()
{
	eco::ContextNodeSet nodes = m_config.find_children("consumer");
	if (nodes.null() || nodes.size() == 0) { return; }

	// #.init consumer that this service depends on.
	const eco::StringAny* v = nullptr;
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		eco::net::TcpClient client;
		client.set_address(init_address(*it));
		init_option(client.get_option(), *it);
		client.get_option().set_module_(client.option().name());
		if (nullptr != (v = it->find("module")))
			client.get_option().set_module_(*v);
		if (nullptr != (v = it->find("suspend")))
			client.get_option().set_suspend(*v);
		if (nullptr != (v = it->find("balance")))
			client.get_option().set_balance(v->c_str());
		if (nullptr != (v = it->find("load_event_sec")))
			client.get_option().set_load_event_sec(*v);
		if (nullptr != (v = it->find("auto_reconnect_sec")))
			client.get_option().set_auto_reconnect_sec(*v);
		m_consumer_set.push_back(client);
	}

	// note: save service address in local to access it by app next time.
	// and it will avoid app can't start up when ops has error.
	// 1) get router service;
	// 2) if step 1 fail, get cs service by local storage.
}
inline void App::Impl::start_consumer()
{
	if (!m_consumer_set.empty())
	{
		/* connect to service to access business data.
		1.sync connect instead of async for app start normally, else it will
		fail when app access data from this consumer.
		*/
		for (auto it = m_consumer_set.begin(); it != m_consumer_set.end(); ++it)
		{
			net::TcpClient& c = *it;
			if (!c.get_option().suspend())
			{
				c.connect(2000);	// sync connect.
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_provider()
{
	// #.create a new tcp server.
	if (!m_config.has("provider"))
	{
		return;
	}
	
	// #.init provider service option.
	const eco::StringAny* v = nullptr;
	eco::net::TcpServerOption& option = m_provider.get_option();
	eco::ContextNode node = m_config.get_node("provider");
	init_option(option, node);
	if (nullptr != (v = node.find("service")))
		set_provider_service(v->c_str());
	if (nullptr != (v = node.find("port")))
		option.set_port(*v);
	if (nullptr != (v = node.find("max_connection_size")))
		option.set_max_connection_size(*v);
	if (nullptr != (v = node.find("max_session_size")))
		option.set_max_session_size(*v);
	if (nullptr != (v = node.find("clean_dos_peer_sec")))
		option.set_clean_dos_peer_sec(*v);
	if (nullptr != (v = node.find("io_thread_size")))
		option.set_io_thread_size(*v);
	if (nullptr != (v = node.find("business_thread_size")))
		option.set_business_thread_size(*v);
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::start_provider()
{
	if (m_provider.get_option().port() != 0)
	{
		m_provider.start();
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_persist()
{
	eco::ContextNodeSet nodes = m_config.find_children("persist");
	if (nodes.null() || nodes.size() == 0)
	{
		return;
	}

	// read persist address(persist).
	eco::persist::AddressSet addr_set;
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		// database/aliyun/property
		eco::persist::Address addr;
		addr.set_name(it->name());
		auto& prop_set = it->get_property_set();
		for (auto p = prop_set.begin(); p != prop_set.end(); ++p)
		{
			if (strcmp(p->name(), "type") == 0)
				addr.set_type(p->get_value().c_str());
			else if (strcmp(p->name(), "user") == 0)
				addr.set_user(p->get_value());
			else if (strcmp(p->name(), "password") == 0)
				addr.set_password(p->get_value());
			else if (strcmp(p->name(), "database") == 0)
				addr.set_database(p->get_value());
			else if (strcmp(p->name(), "address") == 0)
				addr.set_address(p->get_value());
			else if (strcmp(p->name(), "charset") == 0)
				addr.set_charset(p->get_value().c_str());
		}

		// create persist.
		eco::Persist persist;
		persist.set_address(addr);
		m_persist_set.push_back(persist);
	}
}
inline eco::Persist App::Impl::add_persist(eco::persist::Address& addr)
{
	eco::Persist persist;
	persist.set_address(addr);
	m_persist_set.push_back(persist);
	return m_persist_set.back();
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::start_persist()
{
	if (!m_persist_set.empty())
	{
		auto it = m_persist_set.begin();
		for (; it != m_persist_set.end(); ++it)
		{
			it->start();
		}
	}
}


//##############################################################################
//##############################################################################
ECO_SINGLETON_IMPL(App);
void App::set_name(IN const char* val)
{
	impl().set_name(val);
}
App& App::name(IN const char* val) 
{
	impl().set_name(val);
	return *this;
}
const char* App::name() const
{
	return impl().m_name.c_str();
}
void App::set_config_file(IN const char* v)
{
	impl().m_config_file = v;
}
const char* App::config_file() const
{
	return impl().m_config_file.c_str();
}
App& App::get()
{
	return *Impl::s_app;
}
void App::set_app(IN App& app)
{
	if (Impl::s_app == nullptr)
	{
		Impl::s_app = &app;
		Impl::s_app_thread_id = std_this_thread::get_id();
	}
}
uint32_t App::get_param_size()
{
	return (uint32_t)Impl::s_params.size();
}
const char* App::param(IN const int i)
{
	return Impl::s_params.at(i).c_str();
}
const char* App::init_path()
{
	return Impl::s_init_path.c_str();
}
const char* App::exe_path()
{
	return Impl::s_exe_path.c_str();
}
const char* App::exe_file()
{
	return Impl::s_exe_path.c_str();
}
const char* App::module_path() const
{
	return impl().m_module_path.c_str();
}
const char* App::module_file() const
{
	return impl().m_module_file.c_str();
}
void App::get_module_fullpath(OUT std::string& path)
{
	// absolute path judge by: "c:\code\pe\0.5.0\bin".
	size_t pos = path.find_first_of(':');
	if (pos != std::string::npos) return;
	path = module_path() + path;
}


////////////////////////////////////////////////////////////////////////////////
const eco::Config& App::config() const
{
	return impl().m_config;
}
void App::set_log_file_on_changed(IN eco::log::OnChangedLogFile func)
{
	eco::log::Core::get().set_file_on_create(func);
}
eco::cmd::Group App::home()
{
	return eco::cmd::engine().home();
}
eco::Timing& App::timing()
{
	return Eco::get().timing();
}
eco::loc::Locale& App::locale()
{
	return impl().m_locale;
}
eco::net::TcpServer& App::provider()
{
	return impl().m_provider;
}
net::TcpClient App::find_consumer(IN const char* name_)
{
	auto it = impl().m_consumer_set.begin();
	for (; it != impl().m_consumer_set.end(); ++it)
	{
		if (strcmp(it->name(), name_) == 0)
			return *it;
	}
	return eco::null;
}
net::TcpClient App::consumer(IN const char* name_)
{
	auto item = find_consumer(name_);
	if (item.null())
	{
		ECO_THROW(name_) < " consumer isn't exist.";
	}
	return item;
}
uint32_t App::consumer_size()
{
	return (uint32_t)impl().m_consumer_set.size();
}
net::TcpClient App::consumer(IN uint32_t pos)
{
	return impl().m_consumer_set[pos];
}
net::TcpClient App::add_consumer(IN eco::net::Address& addr)
{
	return impl().add_consumer(addr);
}
net::TcpClient App::add_consumer(IN eco::net::AddressSet& addr)
{
	return impl().add_consumer(addr);
}
bool App::ready() const
{
	if (impl().m_state.none())
	{
		return false;
	}
	if (impl().m_state.has(eco::atomic::State::_b))
	{
		return true;
	}
	if (impl().m_state.has(eco::atomic::State::_a))
	{
		if (!impl().ready()) return false;
		impl().m_state.add(eco::atomic::State::_b);
		return true;
	}
	return false;
}
bool app_ready()
{
	return eco::App::get().ready();
}


////////////////////////////////////////////////////////////////////////////////
eco::Persist App::persist(IN const char* name_)
{
	eco::Persist result = find_persist(name_);
	if (result.null()) ECO_THROW("persist set is empty.");
	return result;
}
eco::Persist App::find_persist(IN const char* name_)
{
	if (name_ == nullptr)
	{
		if (!impl().m_persist_set.empty())
		{
			return impl().m_persist_set[0];
		}
	}
	auto it = impl().m_persist_set.begin();
	for (; it != impl().m_persist_set.end(); ++it)
	{
		if (strcmp(it->name(), name_) == 0)
		{
			return *it;
		}
	}
	return eco::null;
}
eco::Persist App::add_persist(IN eco::persist::Address& addr)
{
	return impl().add_persist(addr);
}


////////////////////////////////////////////////////////////////////////////////
extern "C" ECO_API void init_argv(int argc, char* argv[])
{
	// init main parameters.
	for (int i = 0; i < argc; ++i)
	{
		std::string param(argv[i]);
		App::Impl::s_params.push_back(param);
	}

	// init_path & exe_path.
	App::Impl::s_init_path = boost::filesystem::current_path().string();
	App::Impl::s_exe_file = eco::sys::GetAppFile();
	App::Impl::s_exe_path = boost::filesystem::path(App::Impl::s_exe_file)
		.parent_path().string();
	App::Impl::s_exe_path += '\\';
	App::Impl::s_init_path += '\\';
}
extern "C" ECO_API void init_app(IN App& app, IN void* module_func_addr)
{
	app.impl().init(&app, module_func_addr);
}
extern "C" ECO_API void load_app(IN App& app)
{
	app.impl().load(app);
}
extern "C" ECO_API void exit_app(IN App& app)
{
	if (App::Impl::s_app == &app)
	{
		app.impl().exit(app, false);
	}
}


////////////////////////////////////////////////////////////////////////////////
void App::Impl::wait_master(App& app)
{
	{
		std_lock_guard lock(s_monitor.mutex());
		set_app(app);
	}
	if (!master(app) && s_app_thread_id != std_this_thread::get_id())
	{
		if (!s_monitor.wait())	// master fail.
		{
			ECO_THROW("wait master fail.");
		}
	}
	++s_app_counts;				// count apps: master & slaves.
}
void App::Impl::wait_slaves(App& app)
{
	--s_app_counts;
}


////////////////////////////////////////////////////////////////////////////////
void App::Impl::init(App* app, void* module_func_addr)
{
	wait_master(*app);

	// init module path: app or dll.
	m_module_file = eco::sys::GetModuleFile(module_func_addr);
	m_module_path = boost::filesystem::path(
		m_module_file).parent_path().string();
	m_module_path += '\\';

	// #.init system config.
	if (!m_config_file.empty())
	{
		std::string file(m_module_path);
		file += m_config_file;
		m_config.init(file.c_str());
	}
	
	// #.init app config.
	eco::StringAny v;
	if (m_config.find(v, "config/name"))
		set_name(v.c_str());
	
	// #.init logging & eco.
	if (master(*app))
	{
		init_log();
		init_eco();
	}

	// #.init locale.
	init_locale();

	// #.init service.
	init_router();
	init_consumer();
	init_provider();

	// #.init persist.
	init_persist();

	// first init platform, then plugin on platform.
	app->on_init();
	on_rx_init();
}


////////////////////////////////////////////////////////////////////////////////
void App::Impl::load(IN App& app)
{
	// start persist & service.
	start_persist();
	start_consumer();

	app.on_load();
	on_rx_load();

	// start provider at last.
	start_provider();

	if (master(app)) s_monitor.finish();

	m_state.add(eco::atomic::State::_a);
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::exit(IN App& app, IN bool error)
{
	wait_slaves(app);

	if (!error)
	{
		app.to_exit();				// #0.before app exit.
	}

	/* clear resource and data when app exit or has exception.
	release order depend on it's dependency relationship.
	1.eco obj: persist.on_live/erx.on_live <- erx;
	2.logging <- persist <- consumer <- provider <- erx.on_exit;
	3.task server <- all object; but task server can close first.
	*/
	Eco::get().impl().stop();		// #1.async work: eco/timer/task_server.

	if (!m_provider.null())			// #2.provider(tcp server).
	{
		m_provider.stop();
	}

	if (!m_consumer_set.empty())	// #3.consumer(tcp client).
	{
		for (auto it = m_consumer_set.begin(); it != m_consumer_set.end(); ++it)
		{
			it->release();
		}
	}

	if (!m_persist_set.empty())		// #4.persist.
	{
		auto it = m_persist_set.begin();
		for (; it != m_persist_set.end(); ++it)
		{
			it->close();
		}
	}

	if (!error)
	{
		on_rx_exit();				// #5.erx plugin exit.
		app.on_exit();				// #6.exit app
	}

	if (s_app_counts == 0)
	{
		eco::log::Core::get().stop();// #7.exit log
	}	
}


////////////////////////////////////////////////////////////////////////////////
void App::init(App& app, void* module_func_addr, bool command)
{
	try
	{
		// 1.must wait app object construct over.
		// and it can set the sys config path in "derived app()".

		// 2.init app setting and config.
		init_app(app, module_func_addr);

		// 3.init group and command tree: when dll mode.
		if (command)
		{
			app.on_cmd();
			app.impl().on_rx_cmd();
		}

		// 4.finish app config and start business object, load business data.
		load_app(app);

		// message from console.
		if (command)
		{
			eco::cmd::engine().work();
		}
	}
	catch (std::exception& e)
	{
		eco::cout() << "[error] " << e.what();
		app.impl().exit(app, true);
		getch_exit();
	}
}


////////////////////////////////////////////////////////////////////////////////
std_mutex s_init_once;
bool App::init_once(App& app, void* addr, const char* sys_cfg, bool command)
{
	std_lock_guard lock(s_init_once);
	if (app.impl().m_module_path.empty())
	{
		if (sys_cfg) app.set_config_file(sys_cfg);
		App::init(app, addr, command);
		return true;
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////
void App::exit(IN App& app)
{
	eco::exit_app(app);	
}


////////////////////////////////////////////////////////////////////////////////
int App::main(IN App& app, IN int argc, IN char* argv[])
{
	eco::this_thread::init();

	// create dump file.
#ifdef WIN32
	eco::win::Dump::init();
	eco::win::ConsoleEvent::init();
#endif
	
	// init & exit app.
	init_argv(argc, argv);
	App::init(app, nullptr, true);
	App::exit(app);
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);