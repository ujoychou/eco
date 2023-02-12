#ifndef WEB_ENGINE_VIEW_H
#define WEB_ENGINE_VIEW_H
/*******************************************************************************
@ name
qt web page.

@ function
1.communicate with html client, load html web page using webengine.


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-06-10.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <QtWebEngineWidgets/qwebenginepage.h>
#include <QtWebEngineWidgets/qwebengineview.h>


////////////////////////////////////////////////////////////////////////////////
class WebEngineView : public QWebEngineView
{
	Q_OBJECT
public:
	explicit WebEngineView(QWidget* parent = Q_NULLPTR)
		: QWebEngineView(parent)
	{
		QWebEnginePage* page = new QWebEnginePage(parent);
		QWebEngineView::setPage(page);
	}

	// set website url address.
	void setUrl(const QUrl& url)
	{
		QWebEngineView::page()->setUrl(url);
	}
};


////////////////////////////////////////////////////////////////////////////////
#endif
