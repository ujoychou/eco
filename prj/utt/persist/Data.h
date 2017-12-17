#ifndef ECO_PERSIST_DATA_H
#define ECO_PERSIST_DATA_H
/*******************************************************************************
@ 名称

@ 功能

@ 异常

@ 备注

--------------------------------------------------------------------------------
@ 历史记录 @
@ ujoy modifyed on 2016-10-29

--------------------------------------------------------------------------------
* 版权所有(c) 2015 - 2017, ujoychou, 保留所有权利。

*******************************************************************************/
#include <eco/App.h>
#include <eco/persist/ObjectMapping.h>
#include <eco/meta/Timestamp.h>
#include <eco/meta/Meta.h>


namespace eco{;
namespace persist{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class Stock
{
public:
	std::string m_id;
	std::string m_name;
	int64_t m_group_id;

public:
	typedef std::shared_ptr<Stock> value;

	void SetValue(
		IN const std::string& prop,
		IN const std::string& value)
	{
		if (prop == "id")
			m_id = value;
		else if (prop == "name")
			m_name = value;
		else if (prop == "group_id")
			m_group_id = eco::cast<int64_t>(value);
		m_timestamp.update();
	}

	const std::string GetValue(IN const std::string& prop) const
	{
		if (prop == "id")
			return m_id;
		else if (prop == "name")
			return m_name;
		else if (prop == "group_id")
			return eco::cast<std::string>(m_group_id);
		return "";
	}

	eco::meta::Timestamp& timestamp() const
	{
		return m_timestamp;
	}
	mutable eco::meta::Timestamp m_timestamp;
};


class StockMeta : public eco::Meta<Stock>
{
public:
	// set value.
	virtual void set_value(
		IN const char* prop,
		IN const char* value,
		IN const char* view = nullptr)
	{}

	// get value.
	virtual std::string get_value(
		IN const char* prop,
		IN const char* view = nullptr)
	{
		return "";
	}
};


////////////////////////////////////////////////////////////////////////////////
class StockGroup
{
public:
	uint64_t m_id;
	std::string m_name;
	uint64_t m_pool_id;

public:
	typedef std::shared_ptr<StockGroup> value;

	void SetValue(
		IN const std::string& prop,
		IN const std::string& value)
	{
		if (prop == "id")
			m_id = eco::cast<int64_t>(value);
		else if (prop == "name")
			m_name = value;
		else if (prop == "pool_id")
			m_pool_id = eco::cast<int64_t>(value);
		m_timestamp.update();
	}

	const std::string GetValue(IN const std::string& prop) const
	{
		if (prop == "id")
			return eco::cast<std::string>(m_id);
		else if (prop == "name")
			return m_name;
		else if (prop == "pool_id")
			return eco::cast<std::string>(m_pool_id);
		return "";
	}

	eco::meta::Timestamp& timestamp() const
	{
		return m_timestamp;
	}
	mutable eco::meta::Timestamp m_timestamp;
};


class StockGroupMeta : public eco::Meta<StockGroup>
{
public:
	// set value.
	virtual void set_value(
		IN const char* prop,
		IN const char* value,
		IN const char* view = nullptr)
	{}

	// get value.
	virtual std::string get_value(
		IN const char* prop,
		IN const char* view = nullptr)
	{
		return "";
	}
};


////////////////////////////////////////////////////////////////////////////////
class StockPool
{
public:
	uint64_t m_id;
	std::string m_name;

public:
	typedef std::shared_ptr<StockPool> value;

	void SetValue(
		IN const std::string& prop,
		IN const std::string& value)
	{
		if (prop == "id")
			m_id = eco::cast<uint64_t>(value);
		else if (prop == "name")
			m_name = value;
		m_timestamp.update();
	}

	const std::string GetValue(IN const std::string& prop) const
	{
		if (prop == "id")
			return eco::cast<std::string>(m_id);
		else if (prop == "name")
			return m_name;
		return "";
	}

	eco::meta::Timestamp& timestamp() const
	{
		return m_timestamp;
	}
	mutable eco::meta::Timestamp m_timestamp;
};


class StockPoolMeta : public eco::Meta<StockPool>
{
public:
	// set value.
	virtual void set_value(
		IN const char* prop,
		IN const char* value,
		IN const char* view = nullptr)
	{}

	// get value.
	virtual std::string get_value(
		IN const char* prop,
		IN const char* view = nullptr)
	{
		return "";
	}
};


}}}
#endif