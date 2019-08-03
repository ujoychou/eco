#include "PrecHeader.h"
#include <eco/art/WebSocketServer.h>
////////////////////////////////////////////////////////////////////////////////
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtWebSockets/qwebsocketserver.h>
#include <QtWebChannel/qwebchannel.h>
#include <eco/art/WebSocketTransport.h>



namespace eco {;
namespace art {;
////////////////////////////////////////////////////////////////////////////////
QJsonObject WebSocketServer::getJson(const QString& data)
{
	QJsonParseError e;
	QJsonDocument message = QJsonDocument::fromJson(data.toUtf8(), &e);
	return message.object();
}


////////////////////////////////////////////////////////////////////////////////
bool WebSocketServer::run(const int listen_port)
{
	m_server.reset(new QWebSocketServer(
		QStringLiteral("QWS"), QWebSocketServer::NonSecureMode));
	connect(m_server.get(), &QWebSocketServer::newConnection,
		this, &WebSocketServer::onAccept);

	// server start listen html connect.
	if (!m_server->listen(QHostAddress::LocalHost, listen_port))
	{
		return false;
	}

	// webchannel register "server" for web to call.
	m_channel.reset(new QWebChannel());
	onRegisterObject(*m_channel);
	return true;
}


////////////////////////////////////////////////////////////////////////////////
void WebSocketServer::onAccept()
{
	m_channel->connectTo(new eco::art::WebSocketTransport(
		m_server->nextPendingConnection()));
}


////////////////////////////////////////////////////////////////////////////////
}}// ns