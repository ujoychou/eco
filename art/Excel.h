#ifndef ART_QEXCEL_H
#define ART_QEXCEL_H
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
#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>
#include <eco/Object.h>


QT_BEGIN_NAMESPACE
class QAxObject;
QT_END_NAMESPACE

namespace art {;

////////////////////////////////////////////////////////////////////////////////
class Excel : public eco::Object<Excel>
{
public:
	Excel();
	~Excel();

	// 初始化
	void init();
	void release();

	// 打开文件
	void open(const QString& filepath);

	// 关闭文件
	void close();

	// 读取Excel标签中的数据
	void read(
		QList<QList<QVariant>>& data,
		const QString& sheet,
		const QString& flag_pos);

private:
	std::shared_ptr<QAxObject> m_excel;
	std::shared_ptr<QAxObject> m_books;
	std::shared_ptr<QAxObject> m_book;
	std::shared_ptr<QAxObject> m_sheets;
	//std::shared_ptr<QAxObject> m_active_sheet;
};


////////////////////////////////////////////////////////////////////////////////
};
#endif