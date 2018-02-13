#include "PrecHeader.h"
#include <eco/persist/Persist.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/meta/Meta.h>
#include <eco/DllObject.h>
#include <eco/persist/Database.h>
#include <eco/thread/State.h>
#include <eco/App.h>


namespace eco{;

typedef eco::Database* (*CreateFunc)(void);
////////////////////////////////////////////////////////////////////////////////
eco::Database* create_database(IN const persist::SourceType type)
{
	CreateFunc create_func = nullptr;
	if (type == persist::source_mysql)
	{
		static eco::DllObject mysql;
		mysql.set_dll("", "eco_mysql.dll");
		create_func = mysql.cast_function<CreateFunc>("create");
	}
	else if (type == persist::source_sqlite)
	{
		static eco::DllObject sqlite;
		sqlite.set_dll("", "eco_sqlite.dll");
		create_func = sqlite.cast_function<CreateFunc>("create");
	}
	return (create_func != nullptr) ? create_func() : nullptr;
}


////////////////////////////////////////////////////////////////////////////////
class Upgrade
{
public:
	uint32_t m_version;
	Persist::UpgradeFunc m_func;

	inline Upgrade(IN const uint32_t version, IN Persist::UpgradeFunc func)
		: m_version(version), m_func(func)
	{}

	inline bool operator<(IN const Upgrade& v) const
	{
		return m_version < v.m_version;
	}
};


////////////////////////////////////////////////////////////////////////////////
class Persist::Impl
{
public:
	PersistHandler*			m_handler;			// it can be nullptr.
	eco::Database::ptr		m_master;
	std::vector<Upgrade>	m_upgrade_seq;
	eco::ObjectMapping		m_orm_version;
	eco::atomic::State		m_state;
	persist::AddressSet		m_address_set;

	inline Impl() : m_handler(nullptr) 
	{}

	inline void init(IN Persist& parent)
	{
		parent.set_live_ticks(1);	// 30 second.
		m_orm_version.id("version").table("version");
		m_orm_version.add().property("value").field("version").pk(true).int_type();
		m_orm_version.add().property("module").field("module").pk(true).vchar_small();
		m_orm_version.add().property("update_time").field("update_time").date_time();
	}

	inline void on_live();
};


ECO_OBJECT_IMPL(Persist);
////////////////////////////////////////////////////////////////////////////////
bool Persist::on_born()
{
	if (impl().m_address_set.empty())
	{
		return false;
	}

	if (impl().m_handler != nullptr)
	{
		// 1.init object mapping.(orm object using with meta.)
		impl().m_handler->on_mapping();

		// 2.register upgrade function.
		impl().m_handler->on_upgrade();
		std::sort(impl().m_upgrade_seq.begin(), impl().m_upgrade_seq.end());
	}

	// 3.create database config in "sys.xml".
	eco::persist::Address& addr = impl().m_address_set.at(0);
	impl().m_master.reset(create_database(addr.get_type()));
	return (impl().m_master != nullptr);
}


////////////////////////////////////////////////////////////////////////////////
void Persist::Impl::on_live()
{
	try
	{
		// 4.connect to database server.
		if (!m_master->is_open())
		{
			eco::persist::Address& addr = m_address_set.at(0);
			m_master->open(addr);

			// logging persist detail info.
			char log[128] = { 0 };
			sprintf(log, "\n+[persist %s %s]\n""-open %s by %s(%s) at %s:%d\n",
				addr.get_type_name(), addr.get_name(),
				addr.get_database(), addr.get_user(), addr.get_password(),
				addr.get_host(), addr.get_port());
			EcoInfo << log;
		}

		// 5.upgrade database according to it's current version.
		if (!m_state.has(eco::atomic::State::_a))
		{
			// get database version.
			char cond_sql[64] = { 0 };
			const char* app_name = eco::App::instance().get_name();
			sprintf(cond_sql, "where module='%s' order by %s desc",
				app_name, m_orm_version.find_property("value")->get_field());
			eco::persist::Version version(app_name);
			m_master->create_table(m_orm_version);
			m_master->read<eco::persist::VersionMeta>(
				version, m_orm_version, cond_sql);

			// upgrade database.
			auto it = m_upgrade_seq.begin();
			for (; it != m_upgrade_seq.end(); ++it)
			{
				if (it->m_version > version.m_value)
				{
					// upgrade fail, transaction will rollback it's operation.
					eco::Database::Transaction trans(*m_master);
					it->m_func();
					m_master->save<persist::VersionMeta>(
						version.value(it->m_version), m_orm_version);
					trans.commit();
				}
			}
			m_state.add(eco::atomic::State::_a);
		}

		// 6.init business data from persist.
		if (!m_state.has(eco::atomic::State::_b))
		{
			if (m_handler != nullptr)
			{
				m_handler->on_init();
			}
			m_state.add(eco::atomic::State::_b);
		}
	}
	catch (std::exception& e)
	{
		const char* emsg = e.what();
		EcoLogStr(error, static_cast<uint32_t>(strlen(emsg))) << emsg;
	}
	catch (eco::Error& e)
	{
		EcoError << e;
	}
}


////////////////////////////////////////////////////////////////////////////////
void Persist::on_live()
{
	impl().on_live();
}

void Persist::set_address(eco::persist::AddressSet& v)
{
	impl().m_address_set = v;
}

void Persist::set_field(IN const char* prop, IN const char* field)
{
	auto* map = impl().m_orm_version.find_property(prop);
	if (map != nullptr)
	{
		map->field(field);
	}
}

void Persist::register_handler(IN PersistHandler& h)
{
	impl().m_handler = &h;
	impl().m_handler->m_persist = this;
}

void Persist::register_upgrade(IN const uint32_t ver, IN UpgradeFunc func)
{
	impl().m_upgrade_seq.push_back(Upgrade(ver, func));
}

eco::Database& Persist::master()
{
	return *impl().m_master;
}

void Persist::close()
{
	if (impl().m_master != nullptr)
	{
		impl().m_master->close();
		impl().m_master.reset();
	}

	if (impl().m_handler != nullptr)
	{
		impl().m_handler->on_exit();
		impl().m_handler = nullptr;
	}
}

////////////////////////////////////////////////////////////////////////////////
}