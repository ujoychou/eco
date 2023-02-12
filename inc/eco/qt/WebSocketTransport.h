#ifndef WEB_SOCKET_TRANSPORT_H
#define WEB_SOCKET_TRANSPORT_H
/*******************************************************************************
@ name
web socket transport.

@ function



--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy created on 2016-06-12.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Project.h>
#include <QtWebChannel/qwebchannelabstracttransport.h>
class QWebSocket;



ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(art);
////////////////////////////////////////////////////////////////////////////////
class WebSocketTransport : public QWebChannelAbstractTransport
{
	Q_OBJECT
public:
	explicit WebSocketTransport(QWebSocket* socket);
	virtual ~WebSocketTransport();
	virtual void sendMessage(const QJsonObject& message) Q_DECL_OVERRIDE;

private Q_SLOTS:
	void textMessageReceived(const QString& message);

private:
	QWebSocket *m_socket;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
ECO_NS_END(art);
#endif
