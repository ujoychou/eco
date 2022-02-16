#ifndef ECO_QT_TYPE_H
#define ECO_QT_TYPE_H
/*******************************************************************************
@ 名称
QtType类。

@ 功能
1.Qt的公共类型定义。
2.Qt的公共方法定义。

@ 异常

@ 备注

--------------------------------------------------------------------------------
@ 历史记录 @
@ ujoy modifyed on 2019-10-29

--------------------------------------------------------------------------------
* 版权所有(c) 2019 - 2021, ujoy 保留所有权利。

*******************************************************************************/
#include <eco/rx/RxExport.h>
#include <QtCore/qobject.h>
#include <QtCore/qglobal.h>



QT_BEGIN_NAMESPACE
class QSplitter;
class QWidget;
QT_END_NAMESPACE;
namespace eco {;
namespace qt {;
////////////////////////////////////////////////////////////////////////////////
enum
{
	// bit1: direct 
	direct_horizon		= 0b0,
	direct_vertical		= 0b1,

	// bit2-3: align
	align_left			= 0b000,
	align_middle		= 0b010,
	align_right			= 0b100,
};
typedef uint32_t Style;


// widget hide by splitter drag.
bool hideBySplitter(QSplitter* splitter, QWidget* child);

// get widget global rect.
QRect globalRect(QWidget* target);

// map point from one widget to other.
QPoint mapWidgetPoint(QWidget* from, QPoint point, QWidget* to);

// map rect from one widget to other.
QRect mapWidgetRect(QWidget* from, QRect rect, QWidget* to);

// whether the widget contain a global point.
bool pointInWidget(QWidget* target, const QPoint& point);

// get qvariant string.
QString getString(IN const QVariant& qvar);

// set qwidget sheet style.
void setQssFile(const QString& file);
void setQssFile(QWidget* target, const QString& file);
QString readFile(const QString& file);

// update wideget class name.
void updateClass(QWidget* parent, QWidget* target, const char* clss_name);
////////////////////////////////////////////////////////////////////////////////
// geometry function.
inline int rectAdd(int x, int w)
{
	return (w == 0) ? x : x + w - 1;
}
inline int rectDel(int x, int w)
{
	return (w == 0) ? x : x - w + 1;
}


////////////////////////////////////////////////////////////////////////////////
}};
#endif