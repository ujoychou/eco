#include "PrecHeader.h"
#include <eco/App.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/cmd/Engine.h>
#include <eco/RxApp.h>
#include <eco/net/TcpServer.h>
#include <eco/net/TcpServerOption.h>
#include <eco/service/dev/Cluster.h>
#include <vector>
#include "Eco.h"


namespace eco{;

const char* sys_cfg_file = "cfg.sys.xml";
const char* app_cfg_file = "cfg.app.xml";
////////////////////////////////////////////////////////////////////////////////
class App::Impl
{
public:
	void init(IN App& ap) {}
	std::string m_sys_config_file;
	std::string m_app_config_file;
	eco::Config m_app_config;
	eco::Config m_sys_config;
	std::vector<RxDll> m_erx_set;
	eco::net::TcpServer m_provider;
	eco::Persist m_persist;
	static App::CreateAppFunc s_create_app;
	static std::vector<std::string> s_params;

public:
	inline Impl()
		: m_sys_config_file(sys_cfg_file)
		, m_app_config_file("")
		, m_provider(eco::null)
		, m_persist(eco::null)
	{}

	// exit app and clear provider and consumer.
	inline ~Impl()
	{
		stop();
	}

	// wait command and provider thread.
	inline void join();
	inline void stop();

public:
	// eco cmd
	inline bool enable_eco() const;
	inline bool enable_command() const;
	inline void init_command(IN App& app);

	// erx dll
	inline bool enable_erx() const;
	inline void on_rx_init(IN App& ap);
	inline void on_rx_cmd();
	inline void on_rx_exit();

	// service
	inline bool enable_consumer() const;
	inline void init_consumer();
	inline bool enable_provider() const;
	inline void init_provider();
	inline void start_provider();

	// persist
	inline bool enable_persist() const;
	inline void init_persist();
	inline void start_persist();
};


//##############################################################################
//##############################################################################
inline void App::Impl::join()
{
	if (enable_command())
	{
		eco::cmd::get_engine().join();
	}
}

inline void App::Impl::stop()
{
	if (!m_provider.null())
	{
		m_provider.stop();
	}
	if (enable_consumer())
	{
	}
	on_rx_exit();

	if (enable_eco() && get_eco())
	{
		get_eco()->stop();
	}
	eco::log::get_core().stop();
}


////////////////////////////////////////////////////////////////////////////////
inline bool App::Impl::enable_eco() const
{
	StringAny v;
	return m_sys_config.find(v, "eco");
}

inline bool App::Impl::enable_command() const
{
	StringAny v;
	return m_sys_config.find(v, "command");
}

inline void App::Impl::init_command(IN App& app)
{
	if (enable_command())
	{
		app.on_cmd();
		on_rx_cmd();
		eco::cmd::get_engine().run();
	}
}

inline bool App::Impl::enable_erx() const
{
	StringAny v;
	return m_sys_config.find(v, "erx");
}


////////////////////////////////////////////////////////////////////////////
inline void App::Impl::on_rx_init(IN App& ap)
{
	if (enable_erx())
	{
		// load erx config in config file.
		eco::Context rx_set;
		m_sys_config.get_property_set(rx_set, "erx");

		// load erx dll
		for (auto it = rx_set.begin(); it != rx_set.end(); ++it)
		{
			m_erx_set.push_back(RxDll(it->get_key(), it->get_value()));
		}

		// notify erx to init.
		for (auto it = m_erx_set.begin(); it != m_erx_set.end(); ++it)
		{
			it->on_init_app(&ap);
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
			it->on_init_cmd();
		}
	}
}


////////////////////////////////////////////////////////////////////////////
inline void App::Impl::on_rx_exit()
{
	if (enable_erx())
	{
		// notify erx to exit.
		for (auto it = m_erx_set.begin(); it != m_erx_set.end(); ++it)
		{
			if (it->get_cur_message() >= rx_msg_init_app)
			{
				it->on_exit_app();
			}
		}

		// unload erx.
		m_erx_set.clear();
	}
}


////////////////////////////////////////////////////////////////////////////
inline bool App::Impl::enable_consumer() const
{
	StringAny v;
	return m_sys_config.find(v, "consumer");
}


////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_consumer()
{
	if (!enable_consumer())
	{
		return;
	}

	// #.init router.
	eco::ContextNodeSet nodes;
	std::map<std::string, eco::net::AddressSet> router_map;
	m_sys_config.get_children(nodes, "router");
	for (auto router = nodes.begin(); router != nodes.end(); ++router)
	{
		eco::net::AddressSet addrset;
		auto addr = router->get_children().begin();
		for (; addr != router->get_children().end(); ++addr)
		{
			addrset.add().reset(addr->get_value());
		}
	}

	// #.init the service that this service depends on.
	m_sys_config.get_children(nodes, "customer");
	for (auto customer = nodes.begin(); customer != nodes.end(); ++customer)
	{
		eco::net::AddressSet addrset;
		auto addr = customer->get_children().begin();
		for (; addr != customer->get_children().end(); ++addr)
		{
			if (strcmp(addr->get_name(), "router") == 0)		// router mode.
			{
				addrset.set_service_mode(eco::net::router_mode);
				auto r_addr = router_map.find(addr->get_value());
				if (r_addr == router_map.end())
				{
					EcoThrowError << "router address is invalid: "
						<< addr->get_name() << " " << addr->get_value();
				}
				addrset = r_addr->second;
				break;
			}
			else
			{
				addrset.set_service_mode(eco::net::service_mode);
				addrset.add().reset(addr->get_value());		// direct mode.
			}
		}
	}

	// note: save service address in local to access it by app next time.
	// and it will avoid app can't start up when ops has error.
	// 1) get router service; 2) if step 1 fail, get cs service by local storage.
}


////////////////////////////////////////////////////////////////////////////
inline bool App::Impl::enable_provider() const
{
	StringAny v;
	return m_sys_config.find(v, "provider");
}


////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_provider()
{
	// #.create a new tcp server.
	if (!enable_provider())
	{
		return;
	}
	m_provider = eco::heap;

	// #.init provider service option.
	StringAny v;
	eco::net::TcpServerOption& option = m_provider.option();
	if (m_sys_config.find(v, "provider/router"))
		option.set_router(v.c_str());
	if (m_sys_config.find(v, "provider/service"))
		option.set_name(v.c_str());
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
inline bool App::Impl::enable_persist() const
{
	StringAny v;
	return m_sys_config.find(v, "persist");
}


////////////////////////////////////////////////////////////////////////////
inline void App::Impl::init_persist()
{
	if (!enable_persist())
	{
		return;
	}

	// read persist address(persist).
	eco::persist::AddressSet addr_set;
	eco::ContextNodeSet nodes;
	m_sys_config.get_children(nodes, "persist");
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		// database/aliyun
		auto db = it->get_children().begin();
		for (; db != it->get_children().end(); ++db)
		{
			// database/aliyun/property
			eco::persist::Address& addr = addr_set.add();
			auto& prop_set = db->get_property_set();
			for (auto p = prop_set.begin(); p != prop_set.end(); ++p)
			{
				if (strcmp(p->get_key(), "type"))
					addr.set_type(p->get_value().c_str());
				else if (strcmp(p->get_key(), "user"))
					addr.set_user(p->get_value());
				else if (strcmp(p->get_key(), "password"))
					addr.set_password(p->get_value());
				else if (strcmp(p->get_key(), "database"))
					addr.set_database(p->get_value());
				else if (strcmp(p->get_key(), "address"))
					addr.set(p->get_value());
			}
		}
	}

	// init persist with address set.
	if (!addr_set.empty())
	{
		m_persist.set_address(addr_set);
	}
}


////////////////////////////////////////////////////////////////////////////////
inline void App::Impl::start_persist()
{
	if (!m_persist.null())
	{
		m_persist.born();
	}
}


//##############################################################################
//##############################################################################
ECO_TYPE_IMPL(App);
void App::set_sys_config_file(IN const char* v)
{
	impl().m_sys_config_file = v;
}
const char* App::get_sys_config_file() const
{
	return impl().m_sys_config_file.c_str();
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
eco::Persist& App::persist()
{
	return impl().m_persist;
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
	// app group index is 1; and sys group index is 0;
	return eco::cmd::get_engine().root().group_set().at(1);
}
Timer& App::timer()
{
	return get_eco()->timer();
}
eco::net::TcpServer& App::provider()
{
	return impl().m_provider;
}


////////////////////////////////////////////////////////////////////////////////
std::vector<std::string> App::Impl::s_params;
App::CreateAppFunc App::Impl::s_create_app(nullptr);
extern void init_eco();
extern "C" ECO_API void init_app(IN eco::App& ap)
{
	eco::App::Impl& impl = ap.impl();
	// #.init system config.
	impl.m_sys_config.init(impl.m_sys_config_file);

	// #.init app config.
	eco::StringAny v;
	if (impl.m_app_config_file.empty()) {
		if (impl.m_sys_config.find(v, "config/file_path"))
			impl.m_app_config_file = v.c_str();
		else
			impl.m_app_config_file = app_cfg_file;
	}
	impl.m_app_config.init(impl.m_app_config_file);

	// #.read logging config.
	if (impl.m_sys_config.find(v, "logging/sync"))
		eco::log::get_core().set_sync(v);
	if (impl.m_sys_config.find(v, "logging/level"))
		eco::log::get_core().set_severity_level(v.c_str());
	if (impl.m_sys_config.find(v, "logging/memcache"))
		eco::log::get_core().set_capacity(uint32_t(double(v) * 1024 * 1024));
	if (impl.m_sys_config.find(v, "logging/max_sync_millsecs"))
		eco::log::get_core().set_max_sync_interval(v);
	if (impl.m_sys_config.find(v, "logging/file_sink"))
		eco::log::get_core().add_file_sink(v);
	if (impl.m_sys_config.find(v, "logging/console_sink"))
		eco::log::get_core().add_console_sink(v);
	if (eco::log::get_core().has_file_sink())
	{
		if (impl.m_sys_config.find(v, "logging/file_sink/file_path"))
			eco::log::get_core().set_file_path(v.c_str());
		if (impl.m_sys_config.find(v, "logging/file_sink/roll_size"))
			eco::log::get_core().set_file_roll_size(
				uint32_t(double(v) * 1024 * 1024));
		if (impl.m_sys_config.find(v, "logging/file_sink/flush_inverval"))
			eco::log::get_core().set_flush_interval(v);
	}
	eco::log::get_core().run();

	// #.init eco.
	if (impl.enable_eco())
	{
		init_eco();
		if (impl.m_sys_config.find(v, "eco/being/unit_live_tick"))
			get_eco()->set_unit_live_tick_seconds(v);
		if (impl.m_sys_config.find(v, "eco/task_server_thread_size"))
			get_eco()->set_task_server_thread_size(v);
		get_eco()->start();
	}

	// #.init consumer.
	impl.init_consumer();
	// #.init provider.
	impl.init_provider();

	// #.init persist.
	impl.init_persist();
}


////////////////////////////////////////////////////////////////////////////////
int App::main(IN int argc, IN char* argv[])
{
	eco::this_thread::init();

	try
	{
		// init main parameters.
		for (int i = 0; i < argc; ++i)
		{
			std::string param(argv[i]);
			Impl::s_params.push_back(param);
		}
		std::shared_ptr<App> app(Impl::s_create_app());

		// 1.must wait app object construct over.
		// and it can set the sys config path in "derived app()".
		eco::init_app(*app);

		// init erx that ready for "app on_init" using.
		app->impl().on_rx_init(*app);
		// init app business object.
		app->on_init();
		// init command based on app business object and service.
		app->impl().init_command(*app);

		// start persist database.
		app->impl().start_persist();
		// start service provider.
		app->impl().start_provider();

		// join app.
		app->impl().join();

		// exit application.
		app->on_exit();
		// clear resource and data when has exception.
		// ~app().
	}
	catch (eco::Error& e)
	{
		EcoCout << "[error] " << EcoFmt(e);
		getch_exit();
	}
	catch (std::exception& e)
	{
		EcoCout << "[error] " << e.what();
		getch_exit();
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
}