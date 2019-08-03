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
#include <eco/Being.h>
#include <eco/Config.h>
#include <eco/log/Log.h>
#include <eco/cmd/Group.h>
#include <eco/HeapOperators.h>
#include <eco/net/TcpServer.h>
#include <eco/net/TcpClient.h>
#include <eco/net/Subscriber.h>
#include <eco/thread/Timer.h>
#include <eco/persist/Persist.h>


ECO_NS_BEGIN(eco);
class RxDll;
////////////////////////////////////////////////////////////////////////////////
class ECO_API App : public HeapOperators
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

public:
	// console mode: consle event.
	virtual bool on_console(int event) 
	{
		return true;
	}

public:
	virtual ~App();

	/*@ get app instance.*/
	static App* app();

	/*@ get app instance template.*/
	template<typename app_t>
	static inline app_t* get()
	{
		return static_cast<app_t*>(app());
	}

	/*@ get command param size. it's parse from argc and argv.*/
	static uint32_t get_param_size();

	/*@ get command param string.*/
	static const char* get_param(IN const int i);

	/*@ get app config.*/
	void set_name(IN const char*);
	const char* get_name() const;
	App& name(IN const char*);

	/*@ system config file path, default: "eco.sys.xml"; */
	void set_sys_config_file(IN const char*);
	const char* get_sys_config_file() const;

	/*@ config file path, default: "eco.app.xml"; */
	const char* get_config_file() const;

	/*@ get system config.*/
	const Config& get_sys_config() const;

	/*@ get app config.*/
	const Config& get_config() const;

	/*@ file sink: logging file changed callback.*/
	void set_log_file_on_changed(IN log::OnChangedLogFile func);

	// command home dir.
	cmd::Group home();

	// get consumer.
	uint32_t consumer_size();
	net::TcpClient get_consumer(IN const char* name);
	net::TcpClient find_consumer(IN const char* name);
	net::TcpClient get_consumer(IN uint32_t index = 0);

	// get persist.
	eco::Persist persist(IN const char* name = nullptr);

	// service provider.
	net::TcpServer& provider();

	// eco timer.
	Timer& timer();

	// eco erx
	std::shared_ptr<RxDll> get_erx(IN const char* name);

private:
	friend class Startup;
	// set initiatiate
	static void set_app(IN App& app);

	// will first called by c++ raw main.
	static int  main(IN App& app, IN int argc, IN char* argv[]);

	// app life cycle managed by other app.
	static void init(IN App& app, bool command);
	static void exit(IN App& app);
};


////////////////////////////////////////////////////////////////////////////////
class Startup
{
public:
	// app main mode
	typedef int (*CMainFunc)(int argc, char* argv[]);
	inline Startup(IN App& app, IN CMainFunc main_func)
	{
		App::set_app(app);
		main_func = nullptr;
	}

	inline static int main(IN int argc, IN char* argv[])
	{
		return App::main(*eco::App::app(), argc, argv);
	}

public:
	// dll mode
	inline Startup(IN App& app)
	{
		init(app);
	}

	inline static void init(IN App& app)
	{
		App::set_app(app);
		eco::App::init(app, false);
	}

	inline static void exit(IN App& app)
	{
		eco::App::exit(app);
	}
};
ECO_NS_END(eco);


////////////////////////////////////////////////////////////////////////////////
template<typename MainT>
int main(int argc, char* argv[])
{
	return MainT::main(argc, argv);
}
/*@ eco app declare: implement a app instance.*/
#define ECO_APP(AppClass, AppGet)\
ECO_NAME(AppClass, AppGet)\
const eco::Startup eco_startup(AppGet(), &main<eco::Startup>)


////////////////////////////////////////////////////////////////////////////////
#endif