#include "Pch.h"
#include <eco/qt/QtListBar.h>
////////////////////////////////////////////////////////////////////////////////
#include <QtGui/QMouseEvent>
#include <QtCore/QVariant>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>


ECO_NS_BEGIN(eco);
namespace qt{;
////////////////////////////////////////////////////////////////////////////////
ListBar::ListBar(QWidget* parent, Style style, int space)
	: QFrame(parent), m_style(style), m_space(space)
{
	init();

	// 上一步/下一步图标
	m_last = new Label(QString(" ◀ "), this);
	m_next = new Label(QString(" ▶ "), this);
	m_next->stackUnder(m_last);
	
	// 设置样式
	setProperty("class", "list_bar");
	m_last->setProperty("class", "last");
	m_next->setProperty("class", "next");

	// 事件：上一步
	connect(m_last, SIGNAL(clicked(size_t)), this, SLOT(showLast(size_t)));
	// 事件：下一步
	connect(m_next, SIGNAL(clicked(size_t)), this, SLOT(showNext(size_t)));
}
void ListBar::init()
{
	m_begin = m_end = 0;
	m_last_mode = true;
	m_group_focus.clear();
}

////////////////////////////////////////////////////////////////////////////////
size_t ListBar::addItem(
	const QString& text,
	uint64_t param,
	uint16_t group, 
	bool alone, 
	bool bind_event)
{
	// QLabel设置
	auto label = new Label(text, this, param, group, alone);
	label->setContentsMargins(m_space, 0, m_space, 0);
	label->stackUnder(m_last);
	label->stackUnder(m_next);
	label->show();
	m_items.push_back(label);
	
	// 连接事件
	size_t curr = m_items.size() - 1;
	if (bind_event)
	{
		connect(label, &Label::clicked, this, [=] {	focusItem(curr); });
	}
	update();
	return curr;
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::clear()
{
	for (auto it = m_items.begin(); it != m_items.end(); ++it)
	{
		(**it).close();
		(**it).deleteLater();
	}
	m_items.clear();
	init();

	update();
}
size_t ListBar::size() const
{
	return m_items.size();
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::showLast(size_t param)
{
	m_last_mode = true;
	if (--m_begin < 0) m_begin = 0;
	update();
}
void ListBar::showNext(size_t param)
{
	m_last_mode = false;
	if (++m_end >= m_items.size())
		m_end = m_items.size() - 1;

	update();
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::setRect(const QRect& rect)
{
	m_rect = rect;
}
const QRect ListBar::getRect() const
{
	if (m_rect.width() == 0)
	{
		return rect();
	}
	return m_rect;
}
int ListBar::getWidth(const QLabel* label) const
{
	return label->width();
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::paintEvent(QPaintEvent* evt)
{
	updateHorizen();
}
void ListBar::mouseDoubleClickEvent(QMouseEvent* e)
{
	e->accept();
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::updateHorizen()
{
	// 自动适用内容大小。（在最大宽度范围内）
	QRect rect = getRect();
	int width = autoWidth();
	if (width > 0)
	{
		if (alignLeft())
		{
			rect.setWidth(width);
		}
		else if (alignRight())
		{
			rect.setLeft(eco::qt::rectDel(rect.right(), width));
		}
		else if (alignMiddle())
		{
			int delta = rect.width() - width;
			rect.setLeft(eco::qt::rectAdd(rect.left(),  delta / 2));
			rect.setRight(eco::qt::rectAdd(rect.left(), width));
		}
		rect.setHeight(autoHeight());
	}

	// 元素显示不全：显示上一步/下一步。
	int content_width = getContentWidth();
	if (content_width > rect.width())
	{
		m_last->show();
		m_next->show();
		int pos_last_e = eco::qt::rectAdd(rect.left(), getWidth(m_last));
		int pos_next_s = eco::qt::rectDel(rect.right(), getWidth(m_next));
		m_last->setGeometry(QRect(rect.topLeft(),
			QSize(getWidth(m_last), rect.height())));
		m_next->setGeometry(QRect(QPoint(pos_next_s, rect.top()),
			QSize(getWidth(m_next), rect.height())));
		if (m_last_mode)
		{
			updateFromLeft(QPoint(pos_last_e, rect.top()), pos_next_s);
		}
		else
		{
			updateFromRight(QPoint(pos_next_s, rect.top()), pos_last_e);
		}
	}
	// 元素显示：隐藏上一步/下一步。
	else
	{
		m_last->hide();
		m_next->hide();
		if (alignLeft() || alignMiddle())
		{
			m_begin = 0;
			updateFromLeft(rect.topLeft(), rect.right());
		}
		else if (alignRight())
		{
			m_end = m_items.size() - 1;
			updateFromRight(rect.topRight(), rect.left());
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::updateFromLeft(QPoint& pos, int pos_end)
{
	bool set_v = false;
	for (size_t i = m_begin; i < m_items.size(); ++i)
	{
		int width = getWidth(m_items[i]);
		m_items[i]->setGeometry(QRect(pos, QSize(width, autoHeight())));
		pos.setX(pos.x() + width);
		if (pos.x() >= pos_end && !set_v)
		{
			m_end = i; set_v = true;
		}
	}
}
void ListBar::updateFromRight(QPoint& pos, int pos_start)
{
	bool set_v = false;
	for (int i = m_end; i >= 0; --i)
	{
		int width = getWidth(m_items[i]);
		pos.setX(pos.x() - width);
		m_items[i]->setGeometry(QRect(pos, QSize(width, autoHeight())));
		if (pos.x() <= pos_start && !set_v)
		{
			m_begin = i; set_v = true;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
int ListBar::getContentWidth() const
{
	int size = 0;
	for (auto it = m_items.begin(); it != m_items.end(); ++it)
	{
		size += getWidth(*it);
	}
	return size;
}
int ListBar::autoWidth() const
{
	int width = m_rect.width();
	if (width == 0)
	{
		width = QWidget::width();
	}
	return std::min(getContentWidth(), width);
}
int ListBar::autoHeight() const
{
	int height = m_rect.height();
	if (height == 0)
	{
		height = QWidget::height();
	}
	return height;
}
int ListBar::getContentHeight() const
{
	int size = 0;
	return size;
}


////////////////////////////////////////////////////////////////////////////////
uint64_t ListBar::getParam(size_t index) const
{
	return m_items[index]->param();
}
uint64_t ListBar::getFocusParam(uint16_t group) const
{
	return getParam(getFocus(group));
}
size_t ListBar::getIndex(uint64_t param) const
{
	for (auto i = 0; i < m_items.size(); ++i)
	{
		if (param == m_items[i]->param())
			return i;
	}
	return -1;
}
size_t ListBar::getFocusIndex(uint16_t group) const
{
	return getIndex(getFocus(group));
}
Label* ListBar::getItem(size_t index)
{
	return m_items[index];
}
const Label* ListBar::getItem(size_t index) const
{
	return m_items[index];
}
Label* ListBar::getFocusItem(uint16_t group)
{
	return getItem(getFocus(group));
}
const Label* ListBar::getFocusItem(uint16_t group) const
{
	return getItem(getFocus(group));
}
bool ListBar::hasFocusItem(uint16_t group) const
{
	return getFocus(group) != -1;
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::focusItem(size_t curr)
{
	if (curr >= m_items.size() || curr == -1)
	{
		return;
	}

	Label& label = *m_items[curr];
	size_t last = getFocus(label.group());
	if (last == curr)
	{
		// 1.独立开关
		if (label.alone()) label.switchFocus();
		emit clickItem(curr);
	}
	else
	{
		// 2.切换开关：组内切换原则。
		label.focus(true);
		if (last != -1) m_items[last]->focus(false);
		m_group_focus[label.group()] = curr;
		emit clickItem(curr);
		emit switchItem(curr, last);
	}
}


////////////////////////////////////////////////////////////////////////////////
void ListBar::focusItemByParam(uint64_t param)
{
	auto index = getIndex(param);
	if (index != -1)
	{
		focusItem(index);
	}
}


////////////////////////////////////////////////////////////////////////////////
}};