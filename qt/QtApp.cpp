#include "PrecHeader.h"
#include <eco/qt/QtApp.h>
////////////////////////////////////////////////////////////////////////////////
#include <QtCore/QFile>
#include <QtCore/QtGlobal>
#include <QtCore/QStringList>
#include <QtCore/QIODevice>
#include <QtCore/QProcess>
#include <QtGui/QPaintDevice> 
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <eco/sys/WinDump.h>
#include <eco/sys/WinConsoleEvent.h>
////////////////////////////////////////////////////////////////////////////////

namespace eco{;
extern "C" void init_argv(int argc, char* argv[]);
extern "C" void init_app(IN App& app, IN void* module_func_addr);
extern "C" void load_app(IN App& app);
extern "C" void exit_app(IN App& app);
namespace qt{;
////////////////////////////////////////////////////////////////////////////////
uint32_t AppWork::s_run_once = true;
static inline App& app()
{
	return static_cast<App&>(*eco::App::get());
}

////////////////////////////////////////////////////////////////////////////////
void AppWork::setApp(App& app, bool run_once)
{
	s_run_once = run_once;
	eco::Startup startup(&app, nullptr);	// eco app mode init.
}


////////////////////////////////////////////////////////////////////////////////
int AppWork::main(int argc, char* argv[])
{
	eco::this_thread::init();

	// create dump file.
#ifdef WIN32
	eco::win::Dump::init();
	eco::win::ConsoleEvent::init();
#endif

	try
	{
		// 设置支持GPI
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
		QApplication qt_app(argc, argv);

		// 只运行一个实例
		init_argv(argc, argv);
		if (s_run_once)
		{
			return AppWork::runOnce(qt_app);
		}
		// 可运行多个实例
		else
		{
			return AppWork::run(qt_app);
		}
	}
	catch (const std::exception& e)
	{
		QMessageBox::critical(nullptr, tr("program error"), QString(e.what()));
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
int AppWork::run(QApplication& qt_app)
{
	try
	{
		// 创建与初始化APP对象
		eco::init_app(app(), nullptr);
		eco::load_app(app());

		qt_app.exec();

		// 释放APP对象
		eco::exit_app(app());				// exit 2 eco
	}
	catch (eco::Error& e)
	{
		eco::exit_app(app());
		QMessageBox::critical(nullptr, tr("program error"), QString(e.what()));
	}
	catch (std::exception& e)
	{
		eco::exit_app(app());
		QMessageBox::critical(nullptr, tr("program error"), QString(e.what()));
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
int AppWork::runOnce(QApplication& qt_app)
{
	QString app_srv_name = QApplication::applicationName();
	app_srv_name += "__SRV__";

	// 第一次运行，启动APP服务。（用于监听新实例）
	QLocalSocket socket;
	socket.connectToServer(app_srv_name);
	if (!socket.waitForConnected(500))
	{
		m_local_server.reset(new QLocalServer(this));
		connect(m_local_server.get(), SIGNAL(newConnection()),
			this, SLOT(newLocalSocketConnection()));
		if (!m_local_server->listen(app_srv_name))
		{
			QString err("listen local server failed: ");
			err += app_srv_name;
			throw std::logic_error(err.toStdString());
		}

		// 激活APP主逻辑与主窗口。
		return run(qt_app);
	}

	// 不是第一次运行：发送启动参数后退出。
	QTextStream stream(&socket);
	QStringList args = QCoreApplication::arguments();
	for (QStringList::iterator it = args.begin(); it != args.end(); ++it)
	{
		stream << *it;
	}
	stream.flush();
	socket.waitForBytesWritten();
	qApp->quit();
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
void AppWork::newLocalSocketConnection()
{
	std::auto_ptr<QLocalSocket> socket(
		m_local_server->nextPendingConnection());
	if (socket.get() != nullptr)
	{
		socket->waitForReadyRead(1000);
		QTextStream stream(socket.get());

		// 激活窗口
		app().on_active();
	}
}


////////////////////////////////////////////////////////////////////////////////
void App::restart()
{
	qApp->quit();
	QProcess::startDetached(qApp->applicationFilePath(), QStringList());
}


////////////////////////////////////////////////////////////////////////////////
}};