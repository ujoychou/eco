#ifndef TRADE_PE_WEB_SOCKET_SERVER_H
#define TRADE_PE_WEB_SOCKET_SERVER_H
/*******************************************************************************
@ name
web socket server.

@ function
1.receive web client request: "receiveText".
2.send response to web client: "sendText".


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy created on 2016-06-12.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <QtCore/qobject.h>
#include <memory>



QT_BEGIN_NAMESPACE
class QJsonObject;
class QWebSocketServer;
class QWebChannel;
QT_END_NAMESPACE


namespace eco {;
namespace art {;
////////////////////////////////////////////////////////////////////////////////
class WebSocketServer : public QObject
{
public:
	/*@ 启动WebSocket服务.*/
	bool run(const int listen_port);

	// 解析Json.
	static QJsonObject getJson(const QString& data);

protected:
	// 注册通信对象
	virtual void onRegisterObject(QWebChannel& channel) = 0;

	// 接收新连接
	virtual void onAccept();

private:
	std::shared_ptr<QWebChannel> m_channel;
	std::shared_ptr<QWebSocketServer> m_server;
};


////////////////////////////////////////////////////////////////////////////////
}} // ns
#endif
