#ifndef ECO_QT_LIST_BAR_H
#define ECO_QT_LIST_BAR_H
/*******************************************************************************
@ name
列表栏/导航栏。

@ function
1.罗列系列元素，可选中其中某元素。（品种栏）
2.支持颜色设置：背景色/字体颜色/选中高亮色。
3.若元素超出最大范围，则显示向左向右箭头。
4.支持设置对齐方式：左/中/右。
5.图标支持同时显示文字与图片。
6.支持分组显示。（指标栏）
7.支持开关显示，包括独立开关。（指标栏）


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy created on 2019-04-18.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2019 - 2021, ujoy, reserved all right.

*******************************************************************************/
#include <QtWidgets/QWidget>
#include <eco/qt/QtLabel.h>
#include <unordered_map>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE;
ECO_NS_BEGIN(eco);
namespace qt{;
////////////////////////////////////////////////////////////////////////////////
class ListBar : public QFrame
{
	Q_OBJECT
signals:
	void clickItem(size_t curr);
	void switchItem(size_t curr, size_t last);

public slots:
	// 显示上一个
	void showLast(size_t param);

	// 显示下一个
	void showNext(size_t param);

public:
	// 列表栏
	ListBar(
		QWidget* parent, 
		Style style = align_left,
		int space = 10);

	// 设置布局位置与大小
	void setRect(const QRect& rect);
	const QRect getRect() const;

	// 元素内容宽度
	int getContentWidth() const;

	// 添加列表项：文字。
	size_t addItem(
		IN const QString& text,
		IN uint64_t param = 0,
		IN uint16_t group = 0,
		IN bool alone = false,
		IN bool bind_event = true);

	// 添加列表项：图片。
	size_t addIcon(const QString& icon, uint64_t param = 0);

	// 添加列表项：文字与图片。
	size_t addItem(const QString& icon, const QString& text, Style direct);

	// 清空列表项
	void clear();

	// 获取列表项大小
	size_t size() const;

	// 获取列表项：参数
	uint64_t getParam(size_t index) const;
	uint64_t getFocusParam(uint16_t group = 0) const;

	// 获取列表项下标
	size_t getIndex(uint64_t param) const;
	size_t getFocusIndex(uint16_t group = 0) const;

	// 获取列表项
	Label* getItem(size_t index);
	const Label* getItem(size_t index) const;
	Label* getFocusItem(uint16_t group = 0);
	const Label* getFocusItem(uint16_t group = 0) const;

	// 显示指定项。
	void focusItem(size_t index);

	// 显示指定项。
	void focusItemByParam(uint64_t param);

	// 是否选中
	bool hasFocusItem(uint16_t group = 0) const;

public:
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

	// 间距
	inline int space() const 
	{
		return m_space;
	}

private:
	// 重新绘制
	virtual void paintEvent(QPaintEvent* evt) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* e) override;

	// 重置控件位置
	void updateHorizen();
	void updateFromLeft(QPoint& pos, int pos_end);
	void updateFromRight(QPoint& pos, int pos_start);

	// 自动计算宽度/高度
	int autoWidth() const;
	int autoHeight() const;

	// 获取元素总宽度
	int getWidth(const QLabel* label) const;
	// 获取元素总高度
	int getContentHeight() const;

	// 设置初始值
	void init();

	// 获取该组上一次focus元素。
	inline size_t getFocus(uint16_t group) const
	{
		auto it = m_group_focus.find(group);
		return it != m_group_focus.end() ? it->second : -1;
	}

private:
	Style	m_style;				// 对齐样式
	int		m_space;				// 空间间隔
	QRect   m_rect;					// 当前矩阵
	QLabel* m_last;					// 上一个
	QLabel* m_next;					// 下一个
	
	// 元素内容
	std::vector<Label*> m_items;	// 元素列表
	// 各个组的当前元素
	std::map<uint16_t, size_t> m_group_focus;

    // 标签信息
	bool	m_last_mode;			// 上一个与下一个模式
	int		m_begin;				// 开始
	int		m_end;					// 结束
};


////////////////////////////////////////////////////////////////////////////////
}};
#endif
