#ifndef ART_APP_H
#define ART_APP_H
/*******************************************************************************
@ name
qt ui app.

@ function
1.app life cycle is in main funciton life cycle.
2.also app::run and app::exit life cyle.


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-06-10.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <QtCore/qobject.h>
#include <QtCore/qglobal.h>
#include <eco/App.h>
#include <memory>


QT_BEGIN_NAMESPACE
class QLocalServer;
QT_END_NAMESPACE;

namespace eco{;
namespace art{;
////////////////////////////////////////////////////////////////////////////////
class App : public eco::App
{
	friend class AppWork;
protected:
	// 初始化
	virtual void on_init() override = 0;

	// 加载数据
	virtual void on_load() {}

	// 退出程序
	virtual void on_exit() {}

	// 激活程序
	virtual void on_active() 
	{}
};


////////////////////////////////////////////////////////////////////////////////
class AppWork : public QObject
{
	Q_OBJECT
public:
	typedef App* (*createAppFunc)(void);

	// 运行主函数。
	int  main(int argc, char* argv[]);
	static void setApp(createAppFunc func, bool run_once);

	// 运行主窗口逻辑。
	int run(QApplication& qt_app);

	// 运行主窗口逻辑，并只启动一个实例。
	int runOnce(QApplication& qt_app);

	// 写入日志
	static void log(QString& msg);

private slots:
	// 接受一个新连接。
	void newLocalSocketConnection();

private:
	std::shared_ptr<App> m_app;
	std::shared_ptr<QLocalServer> m_local_server;
};


////////////////////////////////////////////////////////////////////////////////
class Startup
{
public:
	Startup(
		IN AppWork::createAppFunc func, 
		IN eco::Startup::CMainFunc main_func,
		IN bool run_once)
	{
		main_func = nullptr;
		AppWork::setApp(func, run_once);
	}

	inline static int main(IN int argc, IN char* argv[])
	{
		eco::art::AppWork runner;
		return runner.main(argc, argv);
	}
};
}};


////////////////////////////////////////////////////////////////////////////////
#define ECO_ART_APP(AppClass, AppGet, run_once)\
ECO_APP_BASE(AppClass, AppGet)\
const eco::art::Startup eco_art_start_up(\
(eco::art::AppWork::createAppFunc)&eco_create_app, \
&main<int, eco::art::Startup>, run_once)



////////////////////////////////////////////////////////////////////////////////
#endif
