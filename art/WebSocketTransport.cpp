#include "PrecHeader.h"
#include "WebSocketTransport.h"
////////////////////////////////////////////////////////////////////////////////
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtWebSockets/qwebsocket.h>
#include <QtWebSockets/qwebsocketserver.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(art);
////////////////////////////////////////////////////////////////////////////////
WebSocketTransport::WebSocketTransport(QWebSocket* socket)
	: QWebChannelAbstractTransport(socket)
	, m_socket(socket)
{
	connect(m_socket, &QWebSocket::textMessageReceived,
		this, &WebSocketTransport::textMessageReceived);
	connect(m_socket, &QWebSocket::disconnected,
		this, &WebSocketTransport::deleteLater);
}


////////////////////////////////////////////////////////////////////////////////
WebSocketTransport::~WebSocketTransport()
{
	m_socket->deleteLater();
}


////////////////////////////////////////////////////////////////////////////////
void WebSocketTransport::sendMessage(const QJsonObject& message)
{
	QJsonDocument doc(message);
	QString data(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
	m_socket->sendTextMessage(data);
}


////////////////////////////////////////////////////////////////////////////////
void WebSocketTransport::textMessageReceived(const QString& data)
{
	QJsonParseError e;
	QJsonDocument message = QJsonDocument::fromJson(data.toUtf8(), &e);
	if (e.error) 
	{
		qWarning() << "Failed to parse text message as JSON object:" << data
			<< "Error is:" << e.errorString();
		return ;
	}
	else if (!message.isObject()) 
	{
		qWarning() << "Received JSON message that is not an object: " << data;
		return ;
	}
	emit messageReceived(message.object(), this);
}


ECO_NS_END(eco);
ECO_NS_END(art);