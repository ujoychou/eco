#include "PrecHeader.h"
#include <eco/App.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/RxApp.h>
#include <eco/sys/Sys.h>
#include <eco/cmd/Engine.h>
#include <eco/net/TcpServer.h>
#ifdef WIN32
#	include <eco/sys/WinDump.h>
#	include <eco/sys/WinConsoleEvent.h>
#endif
#include <eco/thread/Task.h>
#include <eco/thread/Monitor.h>
#include <eco/filesystem/Path.h>
#include <eco/service/dev/Cluster.h>
#include <boost/filesystem/operations.hpp>
#include <thread>
#include <vector>
#include "Eco.ipp"


ECO_NS_BEGIN(eco);
extern void create_eco();
////////////////////////////////////////////////////////////////////////////////
class App::Impl
{
public:
	static App* s_app;
	static std::thread::id s_app_thread_id;
	static eco::Monitor s_monitor;
	static eco::Atomic<int> s_app_counts;
	static std::vector<std::string> s_params;
	static std::string s_exe_path;
	static std::string s_exe_file;
	static std::string s_init_path;

public:
	App* m_app;
	std::string m_name;
	std::string m_module_file;
	std::string m_module_path;
	eco::Config m_sys_config;
	std::string m_sys_config_file;
	mutable eco::atomic::State m_state;
	std::vector<RxDll::ptr> m_erx_set;
	std::vector<eco::Persist> m_persist_set;
	net::TcpServer m_provider;
	std::vector<net::TcpClient> m_consumer_set;
	std::vector<net::AddressSet> m_router_set;
	
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
		if (eco::empty(eco::cmd::get_engine().home().get_name()))
			eco::cmd::get_engine().home().name(m_name.c_str());
	}

	inline void set_provider_service(IN const char* v)
	{
		m_provider.option().set_name(v);
		if (m_name.empty()) set_name(v);
	}

	inline bool ready() const
	{
		for (const eco::Persist& it : m_persist_set)
		{
			if (!it.ready()) return false;
		}
		for (const eco::net::TcpClient& it : m_consumer_set)
		{
			if (!it.ready()) return false;
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
std::thread::id				App::Impl::s_app_thread_id;
eco::Monitor				App::Impl::s_monitor;
eco::Atomic<int>			App::Impl::s_app_counts;
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
	if (m_sys_config.find(v, "logging/async"))
		eco::log::get_core().set_async(v);
	if (m_sys_config.find(v, "logging/level"))
		eco::log::get_core().set_severity_level(v.c_str());
	if (m_sys_config.find(v, "logging/memcache"))
		eco::log::get_core().set_capacity(int(double(v) * 1024 * 1024));
	if (m_sys_config.find(v, "logging/async_flush"))
		eco::log::get_core().set_async_flush(v);
	if (m_sys_config.has("logging/file_sink"))
	{
		eco::log::get_core().add_file_sink(true);
		if (m_sys_config.find(v, "logging/file_sink/level"))
			eco::log::get_core().set_severity_level(v.c_str(), 1);
		if (m_sys_config.find(v, "logging/file_sink/file_path"))
			eco::log::get_core().set_file_path(v.c_str());
		if (m_sys_config.find(v, "logging/file_sink/roll_size"))
			eco::log::get_core().set_file_roll_size(
				uint32_t(double(v) * 1024 * 1024));

		// set logging file path.
		std::string file(m_module_path);
		file += eco::log::get_core().get_file_path();
		eco::filesystem::add_path_suffix(file);
		eco::log::get_core().set_file_path(file.c_str());
	}
	else
	{
		eco::log::get_core().add_file_sink(false);
	}

	if (m_sys_config.has("logging/console_sink"))
	{
		eco::log::get_core().add_console_sink(true);
		if (m_sys_config.find(v, "logging/console_sink/level"))
			eco::log::get_core().set_severity_level(v.c_str(), 2);
	}
	else
	{
		eco::log::get_core().add_console_sink(false);
	}
	eco::log::get_core().run();
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_eco()
{
	StringAny v;
	if (m_sys_config.find(v, "eco/being/unit_live_tick"))
		Eco::Impl::set_unit_live_tick_seconds(v);
	if (m_sys_config.find(v, "eco/task_server_thread_size"))
		Eco::Impl::set_task_server_thread_size(v);
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
	eco::ContextNode node = m_sys_config.find_node("locale");
	if (node.null() || !node.has_children()) return;
	const StringAny* any = node.find("default");
	if (any) eco::loc::locale().set_default(any->c_str());

	// language setting.
	eco::ContextNodeSet node_set = node.get_children();
	for (const ContextNode& lang : node_set)
	{
		std::string path = lang.get("path");
		std::string ver = lang.get("version");
		eco::loc::locale().set_language_info(
			lang.get_name(), path.c_str(), ver.c_str());

		eco::ContextNodeSet file_set = lang.get_children();
		if (file_set.null() || file_set.empty()) continue;
		for (const eco::ContextNode& file : file_set)
		{
			if (eco::equal(file.get_name(), "word"))
			{
				std::string f = get_locale_path(path, file.get("file"));
				eco::StringAny m = file.get("module");
				eco::loc::locale().add_word_file(
					lang.get_name(), f.c_str(), m.c_str());
			}
			else if (eco::equal(file.get_name(), "error"))
			{
				std::string f = get_locale_path(path, file.get("file"));
				eco::StringAny m = file.get("module");
				eco::loc::locale().add_error_file(
					lang.get_name(), f.c_str(), m.c_str());
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
inline bool App::Impl::enable_erx() const
{
	return m_sys_config.has("erx");
}
inline void App::Impl::on_rx_init()
{
	if (enable_erx())
	{
		// load erx config in config file.
		eco::Context rx_set;
		m_sys_config.get_property_set(rx_set, "erx");

		// load erx dll
		for (auto it = rx_set.begin(); it != rx_set.end(); ++it)
		{
			RxDll::ptr rx(new RxDll(it->get_value(), it->get_name()));
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
				if ((**it).get_cur_message() >= rx_msg_init)
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
		if (eco::equal((**it).get_name(), name_))
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
	eco::ContextNodeSet nodes = m_sys_config.find_children("router");
	if (nodes.null() || nodes.size() == 0) return;

	// #.init router.
	for (auto r = nodes.begin(); r != nodes.end(); ++r)
	{
		// router/front_router/
		eco::net::AddressSet addr_set;
		addr_set.set_name(r->get_name());
		// router/front_router/sh_dx.
		auto& prop_set = r->get_property_set();
		for (auto it = prop_set.begin(); it != prop_set.end(); ++it)
		{
			addr_set.add().name(it->get_name()).set(it->get_value());
		}
		m_router_set.push_back(addr_set);
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_option(net::TcpOption& option, ContextNode& node)
{
	const eco::StringAny* v = nullptr;
	if (v = node.find("router"))
		option.set_router(v->c_str());
	if (v = node.find("locale"))
		option.set_locale_(*v);
	if (v = node.find("tick_time"))
		option.set_tick_time(*v);
	if (v = node.find("no_delay"))
		option.set_no_delay(*v);
	if (v = node.find("io_heartbeat"))
		option.set_io_heartbeat(*v);
	if (v = node.find("websocket"))
		option.set_websocket(*v);
	if (v = node.find("response_heartbeat"))
		option.set_response_heartbeat(*v);
	if (v = node.find("rhythm_heartbeat"))
		option.set_rhythm_heartbeat(*v);
	if (v = node.find("heartbeat_recv_tick"))
		option.set_heartbeat_recv_tick(*v);
	if (v = node.find("heartbeat_send_tick"))
		option.set_heartbeat_send_tick(*v);
	if (v = node.find("context_capacity"))
		option.set_context_capacity(*v);
	if (v = node.find("send_capacity"))
		option.set_send_capacity(*v);
	if (v = node.find("send_buffer_size"))
		option.set_send_buffer_size(*v);
	if (v = node.find("recv_buffer_size"))
		option.set_recv_buffer_size(*v);
	if (v = node.find("send_low_watermark"))
		option.set_send_low_watermark(*v);
	if (v = node.find("recv_low_watermark"))
		option.set_recv_low_watermark(*v);
	if (v = node.find("max_byte_size"))
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
			ECO_THROW(0) < node.get_name() < " has no address config.";
		}
	}

	eco::net::AddressSet addr_set;
	const StringAny* v = child->find("router");
	if (v != nullptr)
	{
		auto it = std::find_if(m_router_set.begin(), m_router_set.end(),
			[&v](IN const net::AddressSet& a) -> bool {
			return eco::equal(a.get_name(), v->c_str());
		});
		if (it == m_router_set.end())
		{
			ECO_THROW(0) < it->get_name() < " has no router " < v->c_str();
		}
		addr_set.add_copy(*it);
		addr_set.set_mode(eco::net::router_mode);
	}
	else
	{
		auto& props = child->get_property_set();
		for (auto it = props.begin(); it != props.end(); ++it)
		{
			addr_set.add().name(it->get_name()).set(it->get_value());
		}
		addr_set.set_mode(eco::net::service_mode);
	}
	addr_set.set_name(node.get_name());
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
	eco::ContextNodeSet nodes = m_sys_config.find_children("consumer");
	if (nodes.null() || nodes.size() == 0) return;

	// #.init consumer that this service depends on.
	const eco::StringAny* v = 0;
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		eco::net::TcpClient client;
		client.set_address(init_address(*it));
		init_option(client.option(), *it);
		client.option().set_module_(client.get_option().get_name());
		if (v = it->find("module"))
			client.option().set_module_(*v);
		if (v = it->find("locale"))
			client.option().set_locale_(*v);
		if (v = it->find("auto_reconnect_tick"))
			client.option().set_auto_reconnect_tick(*v);
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
			//it->async_connect();
			it->connect(2000);	// sync connect.
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_provider()
{
	// #.create a new tcp server.
	if (!m_sys_config.has("provider"))
	{
		return;
	}
	
	// #.init provider service option.
	const eco::StringAny* v = 0;
	eco::net::TcpServerOption& option = m_provider.option();
	eco::ContextNode provider = m_sys_config.get_node("provider");
	init_option(option, provider);
	if (v = provider.find("service"))
		set_provider_service(v->c_str());
	if (v = provider.find("port"))
		option.set_port(*v);
	if (v = provider.find("max_connection_size"))
		option.set_max_connection_size(*v);
	if (v = provider.find("max_session_size"))
		option.set_max_session_size(*v);
	if (v = provider.find("clean_dos_peer_tick"))
		option.set_clean_dos_peer_tick(*v);
	if (v = provider.find("io_thread_size"))
		option.set_io_thread_size(*v);
	if (v = provider.find("business_thread_size"))
		option.set_business_thread_size(*v);
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::start_provider()
{
	if (m_provider.option().get_port() != 0)
	{
		m_provider.start();
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_persist()
{
	eco::ContextNodeSet nodes = m_sys_config.find_children("persist");
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
		addr.set_name(it->get_name());
		auto& prop_set = it->get_property_set();
		for (auto p = prop_set.begin(); p != prop_set.end(); ++p)
		{
			if (strcmp(p->get_name(), "type") == 0)
				addr.set_type(p->get_value().c_str());
			else if (strcmp(p->get_name(), "user") == 0)
				addr.set_user(p->get_value());
			else if (strcmp(p->get_name(), "password") == 0)
				addr.set_password(p->get_value());
			else if (strcmp(p->get_name(), "database") == 0)
				addr.set_database(p->get_value());
			else if (strcmp(p->get_name(), "address") == 0)
				addr.set_address(p->get_value());
			else if (strcmp(p->get_name(), "charset") == 0)
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
const char* App::get_name() const
{
	return impl().m_name.c_str();
}
void App::set_config_file(IN const char* v)
{
	impl().m_sys_config_file = v;
}
const char* App::get_config_file() const
{
	return impl().m_sys_config_file.c_str();
}
App* App::get()
{
	return Impl::s_app;
}
void App::set_app(IN App& app)
{
	if (Impl::s_app == nullptr)
	{
		Impl::s_app = &app;
		Impl::s_app_thread_id = std::this_thread::get_id();
	}
}
uint32_t App::get_param_size()
{
	return (uint32_t)Impl::s_params.size();
}
const char* App::get_param(IN const int i)
{
	return Impl::s_params.at(i).c_str();
}
const char* App::get_init_path()
{
	return Impl::s_init_path.c_str();
}
const char* App::get_exe_path()
{
	return Impl::s_exe_path.c_str();
}
const char* App::get_exe_file()
{
	return Impl::s_exe_path.c_str();
}
const char* App::get_module_path() const
{
	return impl().m_module_path.c_str();
}
const char* App::get_module_file() const
{
	return impl().m_module_file.c_str();
}
const eco::Config& App::get_config() const
{
	return impl().m_sys_config;
}
void App::set_log_file_on_changed(IN eco::log::OnChangedLogFile func)
{
	eco::log::get_core().set_file_on_create(func);
}
eco::cmd::Group App::home()
{
	return eco::cmd::get_engine().home();
}
TimerServer& App::timer()
{
	return Eco::get().timer();
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
net::TcpClient App::get_consumer(IN const char* name_)
{
	auto item = find_consumer(name_);
	if (item.null())
	{
		ECO_THROW(0) << name_ << " consumer isn't exist.";
	}
	return item;
}
uint32_t App::consumer_size()
{
	return (uint32_t)impl().m_consumer_set.size();
}
net::TcpClient App::get_consumer(IN uint32_t pos)
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
	if (impl().m_state.is_none())
	{
		return false;
	}
	if (impl().m_state.has(eco::atomic::State::_b))
	{
		return true;
	}
	if (impl().m_state.has(eco::atomic::State::_a))
	{
		if (impl().ready())
		{
			impl().m_state.add(eco::atomic::State::_b);
			return true;
		}
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
eco::Persist App::persist(IN const char* name_)
{
	eco::Persist result = find_persist(name_);
	if (result.null())
		ECO_THROW(0) << "persist set is empty.";
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
		eco::Mutex::ScopeLock lock(s_monitor.mutex());
		set_app(app);
	}
	if (!master(app) && s_app_thread_id != std::this_thread::get_id())
	{
		if (!s_monitor.wait())	// master fail.
		{
			ECO_THROW(0, "wait master fail.");
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
	if (!m_sys_config_file.empty())
	{
		std::string file(m_module_path);
		file += m_sys_config_file;
		m_sys_config.init(file.c_str());
	}
	
	// #.init app config.
	eco::StringAny v;
	if (m_sys_config.find(v, "config/name"))
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
	Eco::get().impl().stop();		// #1.async work: eco/being/task server.

	if (!m_provider.null())			// #2.provider(tcp server).
	{
		m_provider.stop();
	}

	if (!m_consumer_set.empty())	// #3.consumer(tcp client).
	{
		auto it = m_consumer_set.begin();
		for (; it != m_consumer_set.end(); ++it)
		{
			it->close();
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
		eco::log::get_core().stop();// #7.exit log
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
			eco::cmd::get_engine().work();
		}
	}
	catch (eco::Error& e)
	{
		EcoCout << "[error] " << e.what();
		app.impl().exit(app, true);
		getch_exit();
	}
	catch (std::exception& e)
	{
		EcoCout << "[error] " << e.what();
		app.impl().exit(app, true);
		getch_exit();
	}
}


////////////////////////////////////////////////////////////////////////////////
std::mutex s_init_once;
bool App::init_once(App& app, void* addr, const char* sys_cfg, bool command)
{
	std::lock_guard<std::mutex> lock(s_init_once);
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