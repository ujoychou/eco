#include "PrecHeader.h"
#include <eco/persist/Persist.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Being.h>
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
		mysql.load("eco_mysql.dll");
		create_func = mysql.cast_function<CreateFunc>("create");
	}
	else if (type == persist::source_sqlite)
	{
		static eco::DllObject sqlite;
		sqlite.load("eco_sqlite.dll");
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
class Persist::Impl : public eco::Being
{
public:
	enum
	{
		state_init	= eco::atomic::State::_a,
		state_load	= eco::atomic::State::_b,
	};

	class Loading
	{
	public:
		inline Loading(uint64_t& id) : m_load_thread_id(id) {}
		inline ~Loading() { m_load_thread_id = 0; }
		uint64_t& m_load_thread_id;
	};

	PersistHandler*			m_handler;
	eco::Database::ptr		m_master;
	std::vector<Upgrade>	m_upgrade_seq;
	eco::ObjectMapping		m_orm_version;
	eco::atomic::State		m_state;
	persist::Address		m_address;
	uint64_t				m_load_thread_id;

	inline Impl() : m_handler(nullptr), m_load_thread_id(0)
	{
		eco::Being::set_name("persist");
	}

	inline void init(IN Persist& parent)
	{
		set_live_seconds(5);
		m_orm_version.id("version").table("_version");
		m_orm_version.add().property("value").field("version").pk().int_type().index();
		m_orm_version.add().property("module").field("module").pk().varchar(150).index();
		m_orm_version.add().property("timestamp").field("timestamp").date_time();
	}

	virtual bool on_born() override;
	virtual void on_live() override;
};
////////////////////////////////////////////////////////////////////////////////
bool Persist::Impl::on_born()
{
	if (m_handler != nullptr)
	{
		// 1.init object mapping.(orm object using with meta.)
		m_handler->on_mapping();

		// 2.register upgrade function.
		m_handler->on_upgrade();

		// 3.init related object.
		m_handler->on_init();
		std::sort(m_upgrade_seq.begin(), m_upgrade_seq.end());
	}

	// 4.create database config in "sys.xml".
	eco::persist::Address& addr = m_address;
	m_master.reset(create_database(addr.get_type()));
	return (m_master != nullptr);
}


////////////////////////////////////////////////////////////////////////////////
void Persist::Impl::on_live()
{
	// 4.connect to database server.
	if (!m_master->is_open())
	{
		eco::persist::Address& addr = m_address;
		m_master->open(addr);

		// logging persist detail info.
		char log[128] = { 0 };
		sprintf(log, "\n+[persist %s %s]\n""-open %s by %s(%s) at %s:%d\n",
			addr.get_type_name(), addr.get_name(),
			addr.get_database(), addr.get_user(), addr.get_password(),
			addr.get_host(), addr.get_port());
		ECO_INFO << log;
	}

	// 5.upgrade database according to it's current version.
	if (!m_state.has(state_init))
	{
		Loading load(m_load_thread_id = eco::this_thread::id());

		// get database version.
		char cond_sql[64] = { 0 };
		const char* app_name = eco::App::get()->get_name();
		sprintf(cond_sql, "where %s='%s' order by %s desc",
			m_orm_version.find_property("module")->get_field(), app_name,
			m_orm_version.find_property("value")->get_field());
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
				eco::persist::Version ver = version;
				m_master->save<persist::VersionMeta>(
					ver.value(it->m_version),
					m_orm_version, eco::meta::stamp_insert);
				trans.commit();
				version = ver;
			}
		}
		m_state.add(state_init);
	}

	// 6.init business data from persist.
	if (!m_state.has(state_load))
	{
		if (m_handler != nullptr)
		{
			Loading load(m_load_thread_id = eco::this_thread::id());
			m_handler->on_load();
		}

		m_state.add(state_load);
		if (m_handler != nullptr)
		{
			m_handler->ok_load();
		}
	}
}


ECO_SHARED_IMPL(Persist);
////////////////////////////////////////////////////////////////////////////////
void Persist::set_address(IN const persist::Address& v)
{
	impl().m_address = v;
	impl().set_name(v.get_name());
}
const persist::Address& Persist::get_address() const
{
	return impl().m_address;
}
void Persist::start()
{
	impl().live();
}
void Persist::set_field(IN const char* prop, IN const char* field)
{
	auto* map = impl().m_orm_version.find_property(prop);
	if (map != nullptr)
	{
		map->field(field);
	}
}
void Persist::set_handler(IN PersistHandler& h)
{
	impl().m_handler = &h;
	impl().m_handler->m_persist = *this;
}
void Persist::set_upgrade(IN const uint32_t ver, IN UpgradeFunc func)
{
	impl().m_upgrade_seq.push_back(Upgrade(ver, func));
}
eco::Database& Persist::master()
{
	if (!impl().m_state.has(Impl::state_load) &&
		eco::this_thread::id() != impl().m_load_thread_id)
	{
		// only persist is unready and not in "on_load".
		// "on_load": load_thread_id == current thread id.
		ECO_THROW(eco::error) << "persist is unready, unload data.";
	}
	return *impl().m_master;
}
void Persist::close()
{
	if (impl().m_handler != nullptr)
	{
		impl().m_handler->to_exit();
	}
	if (impl().m_master != nullptr)
	{
		impl().m_master->close();
		impl().m_master.reset();
	}
	if (impl().m_handler != nullptr)
	{
		impl().m_handler->on_exit();
	}
}
bool Persist::ready() const
{
	return impl().m_state.has(Impl::state_load);
}


////////////////////////////////////////////////////////////////////////////////
}