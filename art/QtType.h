#ifndef ART_QTTYPE_H
#define ART_QTTYPE_H
/*******************************************************************************
@ 名称
	Excel访问类

@ 功能
	1.读取Excel。

@ 异常

@ 备注

--------------------------------------------------------------------------------
@ 历史记录 @
@ ujoy modifyed on 2016-10-29

--------------------------------------------------------------------------------
* 版权所有(c) 2015 - 2017, mrs corp, 保留所有权利。

*******************************************************************************/
#include <eco/Export.h>
#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>
#include <QtCore/qdatetime.h>
#include <QtCore/QCoreApplication>


QT_BEGIN_NAMESPACE
class QAxObject;
QT_END_NAMESPACE

namespace eco {;
namespace art {;

////////////////////////////////////////////////////////////////////////////////
inline QString getString(IN const QVariant& qvar)
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
inline QString getAppDir()
{
	return QCoreApplication::applicationDirPath();
}


////////////////////////////////////////////////////////////////////////////////
}};
#endif