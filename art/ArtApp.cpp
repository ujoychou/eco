#include "PrecHeader.h"
#include <eco/art/ArtApp.h>
////////////////////////////////////////////////////////////////////////////////
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#	include <QtWidgets/QApplication>
#	include <QtWidgets/QMessageBox>
#else
#	include <QApplication>
#	include <QMessageBox>
#endif
#include <QtNetwork/qlocalserver.h>
#include <QtNetwork/qlocalsocket.h>
#include <QtCore/QStringList>
#include <QtCore/qiodevice.h>
#include <QtCore/qfile.h>


namespace eco{;
extern "C" void init_app(IN App& app);
extern "C" void load_app(IN App& app, bool command);
extern "C" void exit_app(IN App& app);
extern "C" void exit_log(IN App& app);
namespace art{;
////////////////////////////////////////////////////////////////////////////////
uint32_t AppWork::s_run_once = true;
static inline App& app()
{
	return static_cast<App&>(*eco::App::app());
}

////////////////////////////////////////////////////////////////////////////////
void AppWork::setApp(App& app, bool run_once)
{
	s_run_once = run_once;
	eco::Startup startup(app, nullptr);
}


////////////////////////////////////////////////////////////////////////////////
int AppWork::main(int argc, char* argv[])
{
	try
	{
		QApplication qt_app(argc, argv);

		// 只运行一个实例
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
		QMessageBox::critical(nullptr, tr("程序错误(main)"), QString(e.what()));
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
int AppWork::run(QApplication& qt_app)
{
	try
	{
		// 创建与初始化APP对象
		eco::init_app(app());				// init 0 log
		app().on_init();					// init 1 app
		// 加载数据
		eco::load_app(app(), false);		// init 2 eco
		app().on_load();

		qt_app.exec();

		// 释放APP对象
		eco::exit_app(app());				// exit 2 eco
		app().on_exit();					// exit 1 app
		exit_log(app());					// exit 0 log
	}
	catch (eco::Error& e)
	{
		eco::exit_app(app());
		QMessageBox::critical(nullptr, tr("程序错误"), QString(e.what()));
	}
	catch (std::exception& e)
	{
		eco::exit_app(app());
		QMessageBox::critical(nullptr, tr("程序错误"), QString(e.what()));
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
void AppWork::log(QString& msg)
{
	QString filepath("logging/log.txt");
	QScopedPointer<QFile> file(new QFile(filepath));
	if (!file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
	{
		return;
	}
	msg += "\n";
	file->write(msg.toUtf8());
	file->close();
}


////////////////////////////////////////////////////////////////////////////////
}};