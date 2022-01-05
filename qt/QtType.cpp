#include "Pch.h"
#include <eco/qt/QtType.h>
////////////////////////////////////////////////////////////////////////////////
#include <QtCore/QFile>
#include <QtCore/QVariant.h>
#include <QtCore/QDateTime>
#include <QtCore/QCoreApplication>
#include <QtWidgets/QStyle>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QApplication>


ECO_NS_BEGIN(eco);
namespace qt{;
////////////////////////////////////////////////////////////////////////////////
bool hideBySplitter(QSplitter* splitter, QWidget* child)
{
	auto index = splitter->indexOf(child);
	auto list = splitter->sizes();
	return list.at(index) == 0;
}


////////////////////////////////////////////////////////////////////////////////
QRect globalRect(QWidget* target)
{
	QRect rct = target->rect();
	auto top_left = target->mapToGlobal(rct.topLeft());
	auto bottom_right = target->mapToGlobal(rct.bottomRight());
	return QRect(top_left, bottom_right);
}
QPoint mapWidgetPoint(QWidget* from, QPoint pt, QWidget* to)
{
	pt = from->mapToGlobal(pt);
	return to->mapFromGlobal(pt);
}
QRect mapWidgetRect(QWidget* from, QRect rect, QWidget* to)
{
	auto top_left = from->mapToGlobal(rect.topLeft());
	auto bottom_right = from->mapToGlobal(rect.bottomRight());
	top_left = to->mapFromGlobal(top_left);
	bottom_right = to->mapFromGlobal(bottom_right);
	return QRect(top_left, bottom_right);
}
bool pointInWidget(QWidget* target, const QPoint& point)
{
	return globalRect(target).contains(point, true);
}


////////////////////////////////////////////////////////////////////////////////
QString getString(IN const QVariant& qvar)
{
	switch (qvar.type())
	{
	case QVariant::Time:
		return qvar.toTime().toString("hh:mm:ss");
	case QVariant::Date:
		return qvar.toDate().toString("yyyy-MM-dd");
	case QVariant::DateTime:
		return qvar.toDateTime().toString("yyyy-MM-dd hh:mm:ss");
	default:
		return qvar.toString();
	}
	return qvar.toString();
}

////////////////////////////////////////////////////////////////////////////////
void setQssFile(QWidget* target, const QString& qss_file)
{
	QString css = target->styleSheet();
	css += readFile(qss_file);
	target->setStyleSheet(css);
}
void setQssFile(const QString& qss_file)
{
	QString css = qApp->styleSheet();
	css += readFile(qss_file);
	qApp->setStyleSheet(css);
}
QString readFile(const QString& filepath)
{
	QString value;
	QFile file(filepath);
	file.open(QFile::ReadOnly);
	if (file.isOpen())
	{
		value = QString(file.readAll());
		file.close();
	}
	return value;
}


////////////////////////////////////////////////////////////////////////////////
void updateClass(QWidget* parent, QWidget* target, const char* clss_name)
{
	target->setProperty("class", clss_name);
	parent->style()->unpolish(target);
	parent->style()->polish(target);
	parent->update();
}


////////////////////////////////////////////////////////////////////////////////
}};