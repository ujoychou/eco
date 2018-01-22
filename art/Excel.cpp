#include "PrecHeader.h"
#include "Excel.h"
////////////////////////////////////////////////////////////////////////////////
#include <ActiveQt/qaxobject.h> 
#include <QtCore/qfileinfo.h>

namespace art {;


////////////////////////////////////////////////////////////////////////////////
Excel::Excel()
{}


////////////////////////////////////////////////////////////////////////////////
Excel::~Excel()
{
	release();
}


////////////////////////////////////////////////////////////////////////////////
void Excel::init()
{
	// 获取Ole对象
	if (m_excel == nullptr)
	{
		m_excel.reset(new QAxObject("Excel.Application"));	// Office程序
		if (m_excel->isNull())
		{
			m_excel->setControl("ET.Application");			// WPS程序
		}
		if (m_excel->isNull())
		{
			QString v(QStringLiteral("无法解析文件: 本机是否安装了Office程序或者WPS程序"));
			EcoThrowError << v.toStdString();
		}
		m_excel->setProperty("Visible", false);
	}

	if (m_books == nullptr)
	{
		m_books.reset(m_excel->querySubObject("Workbooks"));
	}
}


////////////////////////////////////////////////////////////////////////////////
void Excel::release()
{
	close();	// 关闭文件

	// 退出程序
	if (m_excel != nullptr && !m_excel->isNull())
	{
		m_excel->dynamicCall("Quit()");
	}
	m_books.reset();
	m_excel.reset();
}


////////////////////////////////////////////////////////////////////////////////
void Excel::open(const QString& filepath)
{
	init();

	// 打开Excel文件
	QAxObject* m_book = m_books->querySubObject(
		"Open(QString, QVariant)", filepath, 0);
	if (m_book == nullptr || m_book->isNull())
	{
		EcoThrowError << QStringLiteral("打开文件失败: ").toStdString()
			<< QFileInfo(filepath).fileName().toStdString();
	}
	m_sheets.reset(m_book->querySubObject("WorkSheets"));
}


////////////////////////////////////////////////////////////////////////////////
void Excel::close()
{
	m_sheets.reset();

	if (m_book != nullptr && !m_book->isNull())
	{
		m_book->dynamicCall("Close(Boolean)", false);
	}
	m_book.reset();
}


////////////////////////////////////////////////////////////////////////////////
void Excel::read(
	QList<QList<QVariant> >& data,
	const QString& sheet_name,
	const QString& pos_flag)
{
	// 遍历查找Sheet。
	std::shared_ptr<QAxObject> sheet;
	int sheet_size = m_sheets->property("Count").toInt();
	for (int i = 1; i <= sheet_size; ++i)
	{
		sheet.reset(m_sheets->querySubObject("Item(int)", i));
		QString name = sheet->property("Name").toString().trimmed();
		if (sheet_name == name)
		{
			break;
		}
		sheet.reset();
	}
	if (sheet == nullptr)
	{
		EcoThrowError << QStringLiteral("找不到工作簿: ").toStdString()
			<< sheet_name.toStdString();
	}

	// 读取Sheet所有数据
	std::shared_ptr<QAxObject> used_range(sheet->querySubObject("UsedRange"));
	if (used_range == nullptr || used_range->isNull())
	{
		return;
	}
	auto src = used_range->dynamicCall("Value");
	QList<QVariant> rows = src.toList();
	for (auto it = rows.begin(); it != rows.end(); ++it)
	{
		data.push_back(it->toList());
	}

	// 定位到Sheet位置，并过滤掉之前的数据
	if (!pos_flag.isEmpty())
	{
		bool is_find = false;
		for (auto row = data.begin(); row != data.end();)
		{
			for (int col = 0; col < row->size(); ++col)
			{
				if (pos_flag == row->at(col).toString().trimmed())
				{
					is_find = true;
					break;
				}
			}
			// 删除行：包括定位行及之前的行
			row = data.erase(row);
			if (is_find)
			{
				break;
			}
		}// end for.

		if (!is_find)
		{
			EcoThrowError << QStringLiteral("无法定位交易明细，找不到字符串“").toStdString()
				<< pos_flag.toStdString() << QStringLiteral("”。").toStdString();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
};