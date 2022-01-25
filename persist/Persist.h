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
#include <eco/rx/RxApi.h>
#include <eco/meta/Meta.h>
#include <eco/persist/Address.h>
#include <eco/persist/Database.h>
#include <eco/date_time/DateTime.h>


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
	std::string m_timestamp;

public:
	inline Version() : m_value(0)
	{}

	inline Version(IN const char* name_) : m_value(0), m_module(name_)
	{}

	inline operator const uint32_t() const
	{
		return m_value;
	}

	inline Version& value(IN const uint32_t v)
	{
		m_value = v;
		m_timestamp = eco::date_time::Timestamp(eco::date_time::fmt_std);
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

	inline void set_value(
		IN const char* p,
		IN const char* v,
		IN const char* w = nullptr)
	{
		if (strcmp(p, "value") == 0)
			object().m_value = eco::cast<uint32_t>(v);
		else if (strcmp(p, "module") == 0)
			object().m_module = v;
		else if (strcmp(p, "timestamp") == 0)
			object().m_timestamp = v;
	}

	inline std::string get_value(
		IN const char* p,
		IN const char* w = nullptr)
	{
		if (strcmp(p, "value") == 0)
			return eco::Integer<uint32_t>(object().m_value).c_str();
		else if (strcmp(p, "module") == 0)
			return object().m_module;
		else if (strcmp(p, "timestamp") == 0)
			return object().m_timestamp;
		return eco::empty_str;
	}
};
ECO_NS_END(persist);



////////////////////////////////////////////////////////////////////////////////
class PersistHandler;
class ECO_API Persist 
{
	ECO_SHARED_API(Persist);
public:
	// set address set.
	void set_address(IN const persist::Address&);
	const persist::Address& address() const;

	// get persist name.
	inline const char* name() const
	{
		return address().name();
	}

	// set live seconds.
	void set_live_interval(uint16_t sec);

	// get persist state.
	bool ready() const;

	// register persist handler.
	void set_handler(IN PersistHandler&);

	// start persist.
	void start();

	// close and release persist resource. just like database.
	void close();

	// get persist master. when persis is unloaded it will throw error.
	eco::Database& master();

	// register persist upgrade, it will throw exception when has error.
	typedef std::function<void(void)> UpgradeFunc;
	void set_upgrade(IN const uint32_t ver, IN UpgradeFunc func);

	// set version property related field in database table.
	void set_field(IN const char* prop, IN const char* field);
};


////////////////////////////////////////////////////////////////////////////////
class PersistHandler
{
public:
	inline PersistHandler() : m_persist(eco::null)
	{}

	// init persist object.
	inline void start(IN const persist::Address& addr)
	{
		m_persist = eco::heap;
		m_persist.set_handler(*this);
		m_persist.set_address(addr);
		m_persist.start();
	}

	// event: init object relation mapping.
	virtual void on_mapping() {}

	// event: register upgrade function.
	virtual void on_upgrade() {}

	// event: register upgrade function.
	virtual void on_init() {}

	// event: init business data after config database.
	// it will throw exception when has error.
	virtual void on_load() {}

	// event: on_load finished.
	virtual void ok_load() {}

	// event: clear business data before master close.
	virtual void to_exit() {}

	// event: release business data when persist exit.
	virtual void on_exit() {}

	// get persist.
	inline Persist& persist();

	// get persist ready state.
	inline bool ready();

	// get persist master. when persis is unloaded it will throw error.
	inline eco::Database& master();

private:
	friend class Persist;
	Persist m_persist;
};


////////////////////////////////////////////////////////////////////////////////
inline Persist& PersistHandler::persist()
{
	return m_persist;
}
inline eco::Database& PersistHandler::master()
{
	return persist().master();
}
inline bool PersistHandler::ready()
{
	return m_persist.ready();
}
ECO_NS_END(eco);
#endif