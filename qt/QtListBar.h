#ifndef ECO_QT_LIST_BAR_H
#define ECO_QT_LIST_BAR_H
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
#include <QtWidgets/QWidget>
#include <eco/qt/QtLabel.h>


QT_BEGIN_NAMESPACE
QT_END_NAMESPACE;
namespace eco{;
namespace qt{;
////////////////////////////////////////////////////////////////////////////////
class ListBar : public QWidget
{
	Q_OBJECT
signals:
	void switchItem(size_t curr, size_t last);

public slots:
	// 显示上一个
	void showLast();

	// 显示下一个
	void showNext();

public:
	// 列表栏
	ListBar(QWidget* parent, Style style = direct_horizon | align_left);

	// 添加列表项：文字。
	void addItem(const QString& text, size_t param = 0);

	// 添加列表项：图片。
	void addIcon(const QString& icon, size_t param = 0);

	// 添加列表项：文字与图片。
	void addItem(const QString& icon, const QString& text, Style direct);

public:
	// 水平模式
	inline bool horizon() const
	{
		return (m_style & 0b1) == qt::direct_horizon;
	}

	// 垂直模式
	inline bool vertical() const
	{
		return (m_style & 0b1) == qt::direct_vertical;
	}

	// 左对齐
	inline bool alignLeft() const
	{
		return (m_style & 0b110) == qt::align_left;
	}

	// 右对齐
	inline bool alignRight() const
	{
		return (m_style & 0b110) == qt::align_right;
	}

	// 居中
	inline bool alignMiddle() const
	{
		return (m_style & 0b110) == qt::align_middle;
	}

private:
	// 重新绘制
	virtual void paintEvent(QPaintEvent* evt) override;
	// 重置控件位置
	void updateHorizen();
	void updateVertical();
	void updateFromLeft(QPoint& pos, int pos_end);
	void updateFromRight(QPoint& pos, int pos_start);

	// 获取元素总宽度
	int getContentWidth() const;
	// 获取元素总高度
	int getContentHeight() const;

private:
	Style m_style;
	QLabel* m_last;
	QLabel* m_next;
	Label* m_curr;
	// 元素信息
	std::vector<Label*> m_items;
	bool m_last_mode;
	int m_begin;
	int m_end;
};


////////////////////////////////////////////////////////////////////////////////
}};
#endif
