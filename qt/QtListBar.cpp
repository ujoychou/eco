#include "PrecHeader.h"
#include <eco/qt/QtListBar.h>
////////////////////////////////////////////////////////////////////////////////
#include <QtCore/QVariant>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>


namespace eco{;
namespace qt{;
////////////////////////////////////////////////////////////////////////////////
ListBar::ListBar(QWidget* parent, Style style) : QWidget(parent), m_style(style)
{
	m_begin = m_end = 0;
	m_last_mode = true;

	// 上一步/下一步图标
	m_curr = nullptr;
	m_last = new Label(QString("◀"), this);
	m_next = new Label(QString("▶"), this);
	m_last->stackUnder(m_next);
	
	// 设置样式
	m_last->setProperty("class", "Last");
	m_next->setProperty("class", "Next");
	QWidget::setProperty("class", "ListBar");
	qt::setQssFile(this, "qss/ListBar.css");

	// 事件：上一步
	connect(m_last, SIGNAL(clicked()), this, SLOT(showLast()));
	// 事件：下一步
	connect(m_next, SIGNAL(clicked()), this, SLOT(showNext()));
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::addItem(const QString& text, size_t param)
{
	auto label = new Label(text, this, param);
	label->setFocusPolicy(Qt::ClickFocus);
	label->stackUnder(m_last);
	label->stackUnder(m_next);
	m_items.push_back(label);
	connect(label, &Label::clicked, this, [=] {
		size_t last = m_curr ? m_curr->param() : 0;
		m_curr = label;
		emit switchItem(label->param(), last);
	});

	update();
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::showLast()
{
	m_last_mode = true;
	if (--m_begin < 0)
		m_begin = 0;

	update();
}
void ListBar::showNext()
{
	m_last_mode = false;
	if (++m_end >= m_items.size())
		m_end = m_items.size() - 1;

	update();
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::paintEvent(QPaintEvent* evt)
{
	if (horizon())
	{
		updateHorizen();
	}
	else
	{
		updateVertical();
	}
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::updateHorizen()
{
	QRect rct = rect();
	int content_width = getContentWidth();

	// 元素显示不全：显示上一步/下一步。
	if (content_width > rct.width())
	{
		m_last->move(QPoint(rct.left(), rct.top()));
		m_next->move(QPoint(rct.right() - m_next->width(), rct.top()));
		if (m_last_mode)
		{
			updateFromLeft(
				QPoint(rct.left() + m_last->width(), rct.top()),
				rct.right() - m_next->width());
		}
		else
		{
			updateFromRight(QPoint(rct.right() - m_next->width(), rct.top()),
				rct.left() + m_last->width());
		}
	}
	// 元素显示：隐藏上一步/下一步。
	else
	{
		m_last->move(QPoint(rct.right() + 1, rct.top()));
		m_next->move(QPoint(rct.right() + 1, rct.top()));
		if (alignLeft())
		{
			m_begin = 0;
			updateFromLeft(QPoint(rct.left(), rct.top()), rct.right());
		}
		else if (alignRight())
		{
			m_end = m_items.size() - 1;
			updateFromRight(QPoint(rct.right(), rct.top()), rct.left());
		}
		else if (alignMiddle())
		{
			m_begin = 0;
			int pos_x = rct.left() + (rct.width() - content_width) / 2;
			updateFromLeft(QPoint(pos_x, rct.top()), rct.right());
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::updateVertical()
{
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::updateFromLeft(QPoint& pos, int pos_end)
{
	bool find_it = false;
	for (size_t i = m_begin; i < m_items.size(); ++i)
	{
		m_items[i]->move(pos);
		pos.setX(pos.x() + m_items[i]->width());
		if (pos.x() >= pos_end && !find_it)
		{
			m_end = i;
			find_it = true;
		}
	}
}
void ListBar::updateFromRight(QPoint& pos, int pos_start)
{
	bool find_it = false;
	for (int i = m_end; i >= 0; --i)
	{
		pos.setX(pos.x() - m_items[i]->width());
		m_items[i]->move(pos);
		if (pos.x() <= pos_start && !find_it)
		{
			m_begin = i;
			find_it = true;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
int ListBar::getContentWidth() const
{
	int size = 0;
	std::for_each(m_items.begin(), m_items.end(), [&](const QLabel* it) {
		size += it->width();
	});
	return size;
}
int ListBar::getContentHeight() const
{
	int size = 0;
	std::for_each(m_items.begin(), m_items.end(), [&](const QLabel* it) {
		size += it->height();
	});
	return size;
}


////////////////////////////////////////////////////////////////////////////////
}};