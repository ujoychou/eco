#ifndef ECO_PERSIST_H
#define ECO_PERSIST_H
/*******************************************************************************
@ name


@ function
using persist in two way:
1.implement persist handler, and access handler.persist();
2.use app.persist() directly, no using upgrade solution and default orm & meta.


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/Being.h>
#include <eco/meta/Meta.h>
#include <eco/persist/Address.h>
#include <eco/persist/Database.h>
#include <eco/DateTime.h>


////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(persist);


////////////////////////////////////////////////////////////////////////////////
// database upgrade solution.
class Version : public eco::Value<Version>
{
public:
	uint32_t	m_value;
	std::string m_module;
	std::string m_update_time;
	eco::meta::Timestamp m_timestamp;

public:
	inline Version() : m_timestamp(eco::meta::v_insert), m_value(0)
	{}

	inline Version(IN const char* name) : m_value(0), m_module(name)
	{}

	inline operator const uint32_t() const
	{
		return m_value;
	}

	inline Version& value(IN const uint32_t v, IN bool new_record = true)
	{
		m_value = v;
		m_update_time = eco::date_time::Timestamp(eco::date_time::fmt_std);
		m_timestamp = new_record ? eco::meta::v_insert : eco::meta::v_update;
		return *this;
	}
};
	

////////////////////////////////////////////////////////////////////////////////
class VersionMeta : public eco::Meta<Version>
{
public:
	inline Version create()
	{
		return Version();
	}

	inline eco::meta::Timestamp& timestamp()
	{
		return object().m_timestamp;
	}

	inline void set_value(
		IN const char* p,
		IN const char* v,
		IN const char* w = nullptr)
	{
		if (strcmp(p, "value") == 0)
			object().m_value = eco::cast<uint32_t>(v);
		else if (strcmp(p, "module") == 0)
			object().m_module = v;
		else if (strcmp(p, "update_time") == 0)
			object().m_update_time = v;
	}

	inline std::string get_value(
		IN const char* p,
		IN const char* w = nullptr)
	{
		if (strcmp(p, "value") == 0)
			return eco::Integer<uint32_t>(object().m_value).c_str();
		else if (strcmp(p, "module") == 0)
			return object().m_module;
		else if (strcmp(p, "update_time") == 0) {
			return object().m_update_time;
		}
		return eco::empty_str;
	}
};
ECO_NS_END(persist);


////////////////////////////////////////////////////////////////////////////////
class Persist;
class PersistHandler
{
public:
	// event: init object relation mapping.
	virtual void on_mapping() {}

	// event: register upgrade function.
	virtual void on_upgrade() {}

	// event: init business data after config database.
	// it will throw exception when has error.
	virtual void on_init() {}

	// event: release business data when persist exit.
	virtual void on_exit() {}

	// get persist.
	inline Persist& persist();
	// get persist master.
	inline eco::Database& master();

private:
	friend class Persist;
	Persist* m_persist;
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Persist : public eco::Being
{
	ECO_OBJECT_API(Persist);
public:
	// set address set.
	void set_address(persist::AddressSet&);
	
	// set version property related field in database table.
	void set_field(IN const char* prop, IN const char* field);

	// register persist handler.
	void register_handler(IN PersistHandler&);

	// register persist upgrade, it will throw exception when has error.
	typedef std::function<void(void)> UpgradeFunc;
	void register_upgrade(IN const uint32_t ver, IN UpgradeFunc func);

	// get master data source.
	eco::Database& master();

	// close and release persist resource. just like database.
	void close();

protected:
	virtual bool on_born() override;
	virtual void on_live() override;
};


////////////////////////////////////////////////////////////////////////////////
inline Persist& PersistHandler::persist()
{
	return *m_persist;
}
inline eco::Database& PersistHandler::master()
{
	return persist().master();
}
ECO_NS_END(eco);
#endif