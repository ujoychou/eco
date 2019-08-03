#ifndef ECO_QT_LABEL_H
#define ECO_QT_LABEL_H
/*******************************************************************************
@ name
列表栏/导航栏。

@ function
1.罗列系列元素，可选中其中某元素。
2.支持颜色设置：背景色/字体颜色/选中高亮色。
3.若元素超出最大范围，则显示向左向右箭头。
4.支持设置对齐方式：左/中/右。
5.图标支持同时显示文字与图片。


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy created on 2019-04-18.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2019 - 2021, ujoy, reserved all right.

*******************************************************************************/
#include <QtCore/qobject.h>
#include <QtCore/qglobal.h>
#include <QtWidgets/QLabel>
#include <eco/qt/QtType.h>


QT_BEGIN_NAMESPACE
QT_END_NAMESPACE;
namespace eco{;
namespace qt{;
////////////////////////////////////////////////////////////////////////////////
class Label : public QLabel
{
	Q_OBJECT
signals:
	// 事件：点击鼠标（释放）
	void clicked(size_t param);

public:
	inline Label(
		QWidget* parent = nullptr,
		size_t param = 0,
		Qt::WindowFlags f = Qt::WindowFlags())
		: QLabel(parent, f), m_param(param) {}

	inline Label(
		const QString& text,
		QWidget *parent = nullptr,
		size_t param = 0,
		Qt::WindowFlags f = Qt::WindowFlags())
		: QLabel(text, parent, f), m_param(param) {}

	// 获取ID
	inline size_t param() const
	{
		return m_param;
	}

protected:
	// 点击鼠标（释放）
	virtual void mouseReleaseEvent(QMouseEvent* e) override;
	size_t m_param;
};


////////////////////////////////////////////////////////////////////////////////
}};
#endif
