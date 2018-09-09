#include "PrecHeader.h"
#include <eco/App.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/cmd/Engine.h>
#include <eco/proxy/WinDump.h>
#include <eco/RxApp.h>
#include <eco/net/TcpServer.h>
#include <eco/net/TcpServerOption.h>
#include <eco/service/dev/Cluster.h>
#include <vector>
#include "Eco.h"


namespace eco{;

const char* sys_cfg_file = "eco.sys.xml";
const char* app_cfg_file = "eco.app.xml";
extern void create_eco();
////////////////////////////////////////////////////////////////////////////////
class App::Impl
{
	ECO_IMPL_INIT(App);
public:
	static std::auto_ptr<App> s_app;
	static App::CreateAppFunc s_create_app;
	static std::vector<std::string> s_params;

public:
	std::string m_name;
	std::string m_sys_config_file;
	std::string m_app_config_file;
	eco::Config m_app_config;
	eco::Config m_sys_config;
	std::vector<RxDll::ptr> m_erx_set;
	std::vector<eco::Persist> m_persist_set;
	net::TcpServer m_provider;
	std::vector<net::TcpClient> m_consumer_set;
	std::vector<net::AddressSet> m_router_set;
	
public:
	inline Impl()
		: m_sys_config_file(sys_cfg_file)
		, m_app_config_file("")
		, m_provider(eco::null)
	{}

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

	inline ~Impl()
	{}

	// wait command and provider thread.
	void init();
	void load(bool ui);
	void join();

	// exit app and clear provider and consumer.
	void exit();

public:
	// eco cmd
	inline void init_eco();
	inline void init_command();
	inline void start_command();

	// erx dll
	inline bool enable_erx() const;
	inline void on_rx_init();
	inline void on_rx_cmd();
	inline void on_rx_load();
	inline void on_rx_exit();

	// service
	inline void init_router();
	inline void init_consumer();
	inline void start_consumer();
	inline void init_provider();
	inline void start_provider();
	inline net::AddressSet find_router(IN const char* name);

	// persist
	inline void init_persist();
	inline void start_persist();
};
std::auto_ptr<App> App::Impl::s_app;
std::vector<std::string> App::Impl::s_params;
App::CreateAppFunc App::Impl::s_create_app(nullptr);

//##############################################################################
//##############################################################################
inline void App::Impl::join()
{
	eco::cmd::get_engine().join();
}

inline void App::Impl::exit()
{
	/* release order depend on it's dependency relationship.
	1.eco obj: persist.on_live/erx.on_live <- erx;
	2.logging <- persist <- consumer <- provider <- erx.on_exit;
	3.task server <- all object; but task server can close first.
	*/
	if (get_eco())					// #.1 eco & being & task server.
	{
		get_eco()->stop();
	}

	on_rx_exit();					// #.2 erx plugin exit.

	if (!m_provider.null())			// #.3 provider(tcp server).
	{
		m_provider.stop();
	}

	if (!m_consumer_set.empty())	// #.4 consumer(tcp client).
	{
		auto it = m_consumer_set.begin();
		for (; it != m_consumer_set.end(); ++it)
		{
			it->close();
		}
	}

	if (!m_persist_set.empty())			// #.5 persist.
	{
		auto it = m_persist_set.begin();
		for (; it != m_persist_set.end(); ++it)
		{
			it->close();
		}
	}

	// #.6 logging close after app->on_exit().
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_eco()
{
	if (!get_eco())
	{
		create_eco();
		StringAny v;
		if (m_sys_config.find(v, "eco/being/unit_live_tick"))
			get_eco()->set_unit_live_tick_seconds(v);
		if (m_sys_config.find(v, "eco/task_server_thread_size"))
			get_eco()->set_task_server_thread_size(v);
		// set default value.
		if (get_eco()->get_unit_live_tick_seconds() == 0)
			get_eco()->set_unit_live_tick_seconds(5);
		if (get_eco()->get_unit_live_tick_seconds() == 0)
			get_eco()->set_task_server_thread_size(2);
		get_eco()->start();
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_command()
{
	s_app->on_cmd();
	on_rx_cmd();
}

inline void App::Impl::start_command()
{
	eco::cmd::get_engine().run();
}

inline bool App::Impl::enable_erx() const
{
	return m_sys_config.has("erx");
}


////////////////////////////////////////////////////////////////////////////
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
			RxDll::ptr rx(new RxDll(it->get_key(), it->get_value()));
			m_erx_set.push_back(rx);
		}

		// notify erx to init.
		for (auto it = m_erx_set.begin(); it != m_erx_set.end(); ++it)
		{
			(**it).on_init(*s_app);
		}
	}
}


////////////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////////////
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
	catch (eco::Error& e)
	{
		EcoError << e;
	}
	catch (std::exception& e)
	{
		EcoError << e.what();
	}
}


////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_router()
{
	// #.init router.
	eco::ContextNodeSet nodes = m_sys_config.find_children("router");
	if (!nodes.null())
	{
		for (auto r = nodes.begin(); r != nodes.end(); ++r)
		{
			// router/front_router/
			eco::net::AddressSet addr_set;
			addr_set.set_name(r->get_name());
			// router/front_router/sh_dx.
			auto& prop_set = r->get_property_set();
			for (auto it = prop_set.begin(); it != prop_set.end(); ++it)
			{
				addr_set.add().name(it->get_key()).set(it->get_value());
			}
			m_router_set.push_back(addr_set);
		}
	}
}


////////////////////////////////////////////////////////////////////////////
inline eco::net::AddressSet App::Impl::find_router(IN const char* name)
{
	for (auto it = m_router_set.begin(); it != m_router_set.end(); ++it)
	{
		if (strcmp(it->get_name(), name) == 0)
		{
			return *it;
		}
	}
	return eco::null;
}


////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_consumer()
{
	eco::ContextNodeSet nodes = m_sys_config.find_children("consumer");
	if (nodes.null() || nodes.size() == 0)
	{
		return;
	}

	// #.init consumer that this service depends on.
	StringAny v;
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		// children: consumer address.
		if (!it->has_children() ||
			strcmp(it->get_children().at(0).get_name(), "address") != 0)
		{
			EcoThrow << it->get_name() << " has no address config.";
		}
		eco::net::AddressSet addr_set;
		auto& child = it->get_children().at(0);
		if (child.get_property_set().find(v, "router"))
		{
			auto router = find_router(v.c_str());
			if (router.null() || router.empty())
			{
				EcoThrow << it->get_name() << " has no router " << v.c_str();
			}
			addr_set.add_copy(router);
			addr_set.set_mode(eco::net::router_mode);
		}
		else
		{
			auto& props = child.get_property_set();
			for (auto it = props.begin(); it != props.end(); ++it)
			{
				addr_set.add().name(it->get_key()).set(it->get_value());
			}
			addr_set.set_mode(eco::net::service_mode);
		}
		addr_set.set_name(it->get_name());

		// property_set: consumer config.
		eco::net::TcpClientOption option;
		auto& props = it->get_property_set();
		for (auto it = props.begin(); it != props.end(); ++it)
		{
			if (strcmp(it->get_key(), "no_delay") == 0)
				option.set_no_delay(it->get_value());
			else if (strcmp(it->get_key(), "websocket") == 0)
				option.set_websocket(it->get_value());
			else if (strcmp(it->get_key(), "tick_time") == 0)
				option.set_tick_time(it->get_value());
			else if (strcmp(it->get_key(), "auto_reconnect_tick") == 0)
				option.set_auto_reconnect_tick(it->get_value());
			else if (strcmp(it->get_key(), "heartbeat_recv_tick") == 0)
				option.set_heartbeat_recv_tick(it->get_value());
			else if (strcmp(it->get_key(), "heartbeat_send_tick") == 0)
				option.set_heartbeat_send_tick(it->get_value());
		}

		eco::net::TcpClient client;
		client.set_option(option);
		client.set_address(addr_set);
		m_consumer_set.push_back(client);
	}

	// note: save service address in local to access it by app next time.
	// and it will avoid app can't start up when ops has error.
	// 1) get router service;
	// 2) if step 1 fail, get cs service by local storage.
}


////////////////////////////////////////////////////////////////////////////////
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
			it->connect_wait(2000);
		}
	}
}


////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_provider()
{
	// #.create a new tcp server.
	if (!m_sys_config.has("provider"))
	{
		return;
	}
	
	// #.init provider service option.
	StringAny v;
	m_provider = eco::heap;
	eco::net::TcpServerOption& option = m_provider.option();
	if (m_sys_config.find(v, "provider/router"))
		option.set_router(v.c_str());
	if (m_sys_config.find(v, "provider/service"))
		set_provider_service(v.c_str());
	if (m_sys_config.find(v, "provider/port"))
		option.set_port(v);
	if (m_sys_config.find(v, "provider/tick_time"))
		option.set_tick_time(v);
	if (m_sys_config.find(v, "provider/no_delay"))
		option.set_no_delay(v);
	if (m_sys_config.find(v, "provider/max_connection_size"))
		option.set_max_connection_size(v);
	if (m_sys_config.find(v, "provider/max_session_size"))
		option.set_max_session_size(v);
	if (m_sys_config.find(v, "provider/io_heartbeat"))
		option.set_io_heartbeat(v);
	if (m_sys_config.find(v, "provider/websocket"))
		option.set_websocket(v);
	if (m_sys_config.find(v, "provider/response_heartbeat"))
		option.set_response_heartbeat(v);
	if (m_sys_config.find(v, "provider/rhythm_heartbeat"))
		option.set_rhythm_heartbeat(v);
	if (m_sys_config.find(v, "provider/heartbeat_recv_tick"))
		option.set_heartbeat_recv_tick(v);
	if (m_sys_config.find(v, "provider/heartbeat_send_tick"))
		option.set_heartbeat_send_tick(v);
	if (m_sys_config.find(v, "provider/io_thread_size"))
		option.set_io_thread_size(v);
	if (m_sys_config.find(v, "provider/business_thread_size"))
		option.set_business_thread_size(v);
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::start_provider()
{
	if (!m_provider.null())
	{
		m_provider.start();
	}
}


////////////////////////////////////////////////////////////////////////////
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
			if (strcmp(p->get_key(), "type") == 0)
				addr.set_type(p->get_value().c_str());
			else if (strcmp(p->get_key(), "user") == 0)
				addr.set_user(p->get_value());
			else if (strcmp(p->get_key(), "password") == 0)
				addr.set_password(p->get_value());
			else if (strcmp(p->get_key(), "database") == 0)
				addr.set_database(p->get_value());
			else if (strcmp(p->get_key(), "address") == 0)
				addr.set_address(p->get_value());
			else if (strcmp(p->get_key(), "charset") == 0)
				addr.set_charset(p->get_value().c_str());
		}

		// create persist.
		eco::Persist persist;
		persist.set_address(addr);
		m_persist_set.push_back(persist);
	}

	// init persist with address set.
	if (!m_persist_set.empty())
	{
		init_eco();
	}
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
ECO_TYPE_IMPL(App);
ECO_PROPERTY_STR_IMPL(App, name);
void App::set_sys_config_file(IN const char* v)
{
	impl().m_sys_config_file = v;
}
const char* App::get_sys_config_file() const
{
	return impl().m_sys_config_file.c_str();
}
const char* App::get_config_file() const
{
	return impl().m_app_config_file.c_str();
}
App& App::instance()
{
	return *Impl::s_app;
}
void App::set_create_app_func(IN CreateAppFunc func)
{
	App::Impl::s_create_app = func;
}
uint32_t App::get_param_size()
{
	return (uint32_t)App::Impl::s_params.size();
}
const char* App::get_param(IN const int i)
{
	return App::Impl::s_params.at(i).c_str();
}
const eco::Config& App::get_sys_config() const
{
	return impl().m_sys_config;
}
const eco::Config& App::get_config() const
{
	return impl().m_app_config;
}
void App::set_log_file_on_changed(IN eco::log::OnChangedLogFile func)
{
	eco::log::get_core().set_file_on_create(func);
}
eco::cmd::Group App::home()
{
	return eco::cmd::get_engine().home();
}
Timer& App::timer()
{
	return get_eco()->timer();
}
eco::net::TcpServer& App::provider()
{
	return impl().m_provider;
}
net::TcpClient App::find_consumer(IN const char* name)
{
	auto it = impl().m_consumer_set.begin();
	for (; it != impl().m_consumer_set.end(); ++it)
	{
		if (strcmp(it->get_name(), name) == 0)
			return *it;
	}
	return eco::null;
}
net::TcpClient App::get_consumer(IN const char* name)
{
	auto item = find_consumer(name);
	if (item.null())
	{
		EcoThrow << name << " consumer isn't exist.";
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


////////////////////////////////////////////////////////////////////////////////
eco::Persist App::persist(IN const char* name)
{
	if (name == nullptr)
	{
		if (!impl().m_persist_set.empty())
		{
			return impl().m_persist_set[0];
		}
	}
	auto it = impl().m_persist_set.begin();
	for (; it != impl().m_persist_set.end(); ++it)
	{
		if (strcmp(it->get_name(), name) == 0)
		{
			return *it;
		}
	}
	EcoThrow << "persist set is empty.";
	return eco::null;
}


////////////////////////////////////////////////////////////////////////////////
extern "C" ECO_API void init_app(IN App* app)
{
	App::Impl::s_app.reset(app);
	app->impl().init();
}
extern "C" ECO_API void load_app(IN App& app, bool ui)
{
	app.impl().load(ui);
}
extern "C" ECO_API void exit_app()
{
	App::Impl::s_app->impl().exit();
}
extern "C" ECO_API void exit_log()
{
	eco::log::get_core().stop();
}


////////////////////////////////////////////////////////////////////////////////
void App::Impl::init()
{
	// #.init system config.
	m_sys_config.init(m_sys_config_file);

	// #.init app config.
	eco::StringAny v;
	if (m_app_config_file.empty()) {
		if (m_sys_config.find(v, "config/name"))
			set_name(v.c_str());
		if (m_sys_config.find(v, "config/file_path"))
			m_app_config_file = v.c_str();
		else
			m_app_config_file = app_cfg_file;
	}
	m_app_config.init(m_app_config_file);

	// #.read logging config.
	if (m_sys_config.find(v, "logging/async"))
		eco::log::get_core().set_async(v);
	if (m_sys_config.find(v, "logging/level"))
		eco::log::get_core().set_severity_level(v.c_str());
	if (m_sys_config.find(v, "logging/memcache"))
		eco::log::get_core().set_capacity(uint32_t(double(v) * 1024 * 1024));
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
	}
	if (m_sys_config.has("logging/console_sink"))
	{
		eco::log::get_core().add_console_sink(true);
		if (m_sys_config.find(v, "logging/console_sink/level"))
			eco::log::get_core().set_severity_level(v.c_str(), 2);
	}
	eco::log::get_core().run();

	// #.init eco.
	if (m_sys_config.has("eco"))
	{
		init_eco();
	}

	// #.init service.
	init_router();
	init_consumer();
	init_provider();

	// #.init persist.
	init_persist();

	// init erx that ready for "app on_init" using.
	on_rx_init();
}


////////////////////////////////////////////////////////////////////////////////
void App::Impl::load(bool ui)
{
	// init group and command tree.
	if (!ui)
	{
		init_command();
	}

	// start persist & service.
	start_persist();
	start_consumer();
	start_provider();
	// start command based on app business object and service.
	if (!ui)
	{
		start_command();
	}
	on_rx_load();
}


////////////////////////////////////////////////////////////////////////////////
void App::init(IN int argc, IN char* argv[])
{
	eco::this_thread::init();

#ifdef WIN32
	eco::win::Dump::init();		// create dump file.
#endif

	// init main parameters.
	for (int i = 0; i < argc; ++i)
	{
		std::string param(argv[i]);
		Impl::s_params.push_back(param);
	}

	// 1.must wait app object construct over.
	// and it can set the sys config path in "derived app()".
	eco::init_app(Impl::s_create_app());// init 0 log
	Impl::s_app->on_init();				// init 1 app

	// finish app config and start business object, load app business data.
	Impl::s_app->impl().load(false);	// init 2 eco
	Impl::s_app->on_load();
}


////////////////////////////////////////////////////////////////////////////////
void App::exit()
{
	// clear resource and data when has exception.
	eco::exit_app();					// exit 2 eco

	// exit application.
	if (Impl::s_app.get() != nullptr)
	{
		Impl::s_app->on_exit();			// exit 1 app
	}
	exit_log();							// exit 0 log
}


////////////////////////////////////////////////////////////////////////////////
int App::main(IN int argc, IN char* argv[])
{
	try
	{
		App::init(argc, argv);

		// join app command.
		if (App::Impl::s_app.get() != nullptr)
		{
			Impl::s_app->impl().join();
		}

		App::exit();
	}
	catch (eco::Error& e)
	{
		eco::exit_app();
		EcoCout << "[error] " << e.what();
		getch_exit();
	}
	catch (std::exception& e)
	{
		eco::exit_app();
		EcoCout << "[error] " << e.what();
		getch_exit();
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
}