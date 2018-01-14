#ifndef ECO_PERSIST_H
#define ECO_PERSIST_H
/*******************************************************************************
@ name


@ function


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


////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(persist);


////////////////////////////////////////////////////////////////////////////////
// database upgrade solution.
class Version
{
public:
	uint32_t	m_value;
	std::string m_module;
	std::string m_update_time;
	eco::meta::Timestamp m_timestamp;

public:
	inline operator const uint32_t() const
	{
		return m_value;
	}

	inline Version& value(IN const uint32_t v, IN bool new_record = false)
	{
		m_value = v;
		if (!new_record)
			m_timestamp.update();
		else
			m_timestamp.set_value(eco::meta::inserted);
		return *this;
	}
};
	

////////////////////////////////////////////////////////////////////////////////
class VersionMeta : public eco::Meta<Version>
{
public:
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
	virtual void on_upgrade(OUT Persist& persist) {}

	// event: init business data after config database.
	virtual void on_init(OUT Persist& persist) {}

public:
	// get persist.
	Persist& persist();
};


////////////////////////////////////////////////////////////////////////////////
class Persist : public eco::Being
{
	ECO_OBJECT_API(Persist);
public:
	// set address set.
	void set_address(persist::AddressSet&);

	// register persist handler.
	void register_handler(IN PersistHandler&);

	// register persist upgrade sql.
	typedef bool(*UpgradeFunc)(IN Persist&);
	void register_upgrade(IN const uint32_t ver, IN UpgradeFunc func);

	// get master data source.
	eco::Database& master();

protected:
	virtual bool on_born() override;
	virtual void on_live() override;
};


ECO_NS_END(eco);
#endif