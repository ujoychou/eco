#include "Pch.h"
#include <eco/qt/QtLabel.h>
////////////////////////////////////////////////////////////////////////////////
#include <QMouseEvent>

ECO_NS_BEGIN(eco);
namespace qt{;
////////////////////////////////////////////////////////////////////////////////
void Label::mouseReleaseEvent(QMouseEvent* evt)
{
	QMouseEvent* mevt = static_cast<QMouseEvent*>(evt);
	if (eco::qt::pointInWidget(this, mevt->globalPos()))
	{
		Q_UNUSED(evt);
		emit clicked(m_param);
		evt->accept();
	}
	QLabel::mouseReleaseEvent(evt);
}


////////////////////////////////////////////////////////////////////////////////
void Label::focus(bool is)
{
	if (m_focus != uint16_t(is))
		updateClass(parentWidget(), this, (m_focus = is) ? "focus" : "");
}

////////////////////////////////////////////////////////////////////////////////
}};