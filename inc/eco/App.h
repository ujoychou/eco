#ifndef ECO_APP_H
#define ECO_APP_H
/*******************************************************************************
@ name
app that run process.

@ function
1.access entry of process.
2.provide logging and config.

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Config.h>
#include <eco/log/Log.h>
#include <eco/cmd/Group.h>
#include <eco/loc/Locale.h>
#include <eco/net/TcpServer.h>
#include <eco/net/TcpClient.h>
#include <eco/net/Subscriber.h>
#include <eco/persist/Persist.h>
#include <eco/thread/Timing.h>


ECO_NS_BEGIN(eco);
class RxDll;
////////////////////////////////////////////////////////////////////////////////
class ECO_API App : public RxHeap
{
	ECO_IMPL_API();
	ECO_NONCOPYABLE(App);
protected:
	// this is a base class, must be derived.
	App();

	// init app config.
	virtual void on_init() {}

	// init command.
	virtual void on_cmd() {}

	// load app business data after app init finish.
	virtual void on_load() {}

	// before app exit.
	virtual void to_exit() {}

	// app exit.
	virtual void on_exit() {}

	// console mode: consle event.
	virtual bool on_console(int event) 
	{
		return true;
	}

public:
	virtual ~App();

	/*@ get app instance.*/
	static App& get();

	/*@ get app instance template.*/
	template<typename app_t>
	static inline app_t& cast()
	{
		return static_cast<app_t&>(get());
	}

	/*@ get command param size. it's parse from argc and argv.*/
	static uint32_t get_param_size();

	/*@ get command param string.*/
	static const char* param(IN const int i);
	static const char* init_path();
	static const char* exe_path();
	static const char* exe_file();
	const char* module_path() const;
	const char* module_file() const;

	/*@ get full path of string.*/
	void get_module_fullpath(OUT std::string& v);

	/*@ whether current module is a exe.*/
	inline bool module_exe() const
	{
		return strcmp(exe_file(), module_file()) == 0;
	}
	
	// whether app is ready.
	bool ready() const;

	/*@ get app config.*/
	void set_name(IN const char*);
	const char* name() const;
	App& name(IN const char*);

	/*@ system config file path, default: "eco.sys.xml"; */
	void set_config_file(IN const char*);
	const char* config_file() const;

	/*@ get app config.*/
	const Config& config() const;

	/*@ file sink: logging file changed callback.*/
	void set_log_file_on_changed(IN log::OnChangedLogFile func);

	// command home dir.
	cmd::Group home();

	// get consumer.
	uint32_t consumer_size();
	eco::net::TcpClient consumer(IN const char* name);
	eco::net::TcpClient find_consumer(IN const char* name);
	eco::net::TcpClient consumer(IN uint32_t index = 0);
	eco::net::TcpClient add_consumer(IN eco::net::Address&);
	eco::net::TcpClient add_consumer(IN eco::net::AddressSet&);

	// get persist.
	eco::Persist persist(IN const char* name = nullptr);
	eco::Persist find_persist(IN const char* name = nullptr);
	eco::Persist add_persist(IN eco::persist::Address& addr);

	// service provider.
	net::TcpServer& provider();

	// eco timing wheel.
	eco::Timing& timing();

	// eco erx
	std::shared_ptr<RxDll> get_erx(IN const char* name);

	// eco locale and multi language.
	eco::loc::Locale& locale();

private:
	friend class Startup;
	friend class AppInner;
	// set initiatiate
	static void set_app(IN App& app);

	// will first called by c++ raw main.
	static int  main(IN App& app, IN int argc, IN char* argv[]);

	// app life cycle managed by other app.
	static void init(App& app, void* module_func_addr, bool command);
	static bool init_once(App& app, void* mfa, const char* cfg, bool cmd);
	static void exit(IN App& app);
};


////////////////////////////////////////////////////////////////////////////////
class Startup
{
public:
	// app mode: main function.
	typedef int (*CMainFunc)(int argc, char* argv[]);
	inline Startup(IN App* app, IN CMainFunc)
	{
		App::set_app(*app);
	}

	inline static int main(IN int argc, IN char* argv[])
	{
		return App::main(eco::App::get(), argc, argv);
	}

public:
	// dll mode: but not erx mode.
	// sys_cfg also can be set in derived app constructor.
	inline Startup(
		IN App& app,
		IN void* module_func_addr,
		IN const char* sys_cfg = nullptr)
	{
		init(app, module_func_addr, sys_cfg);
	}

	// init app thread unsafe.
	inline static void init(
		IN App& app,
		IN void* module_func_addr,
		IN const char* sys_cfg = nullptr)
	{
		if (sys_cfg) app.set_config_file(sys_cfg);
		eco::App::init(app, module_func_addr, false);
	}

	// init_once can using "static Startup startup(get_app())"
	inline static bool init_once(
		IN App& app,
		IN void* module_func_addr,
		IN const char* sys_cfg = nullptr)
	{
		return eco::App::init_once(app, module_func_addr, sys_cfg, false);
	}

	inline static void exit(IN App& app)
	{
		eco::App::exit(app);
	}
};
ECO_NS_END(eco);


////////////////////////////////////////////////////////////////////////////////
template<typename MainT>
int maint(int argc, char* argv[])
{
	return MainT::main(argc, argv);
}
/*@ eco app declare: implement a app instance.*/
#define ECO_APP(AppClass, AppGet)\
ECO_NAME(AppClass, AppGet)\
const eco::Startup eco_startup(&AppGet(), &maint<eco::Startup>)


////////////////////////////////////////////////////////////////////////////////
#endif