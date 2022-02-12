#include "Pch.h"
#include <eco/persist/Persist.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/meta/Meta.h>
#include <eco/rx/RxObject.h>
#include <eco/persist/Database.h>
#include <eco/thread/State.h>
#include <eco/App.h>


ECO_NS_BEGIN(eco);
typedef eco::Database* (*CreateFunc)(void);
////////////////////////////////////////////////////////////////////////////////
eco::Database* create_database(IN const persist::SourceType type)
{
	CreateFunc create_func = nullptr;
	if (type == persist::source_mysql)
	{
		static eco::RxObject mysql;
		mysql.load("eco_mysql.dll");
		create_func = mysql.cast_func<CreateFunc>("create");
	}
	else if (type == persist::source_sqlite)
	{
		static eco::RxObject sqlite;
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
class Persist::Impl
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

	Timing::Timer		m_timer;
	PersistHandler*			m_handler;
	eco::Database::ptr		m_master;
	persist::Address		m_address;
	std::vector<Upgrade>	m_upgrade_seq;
	eco::ObjectMapping		m_orm_version;
	eco::atomic::State		m_state;
	uint64_t				m_load_thread_id;
	uint32_t				m_live_interval;

	inline Impl() : m_handler(nullptr), m_load_thread_id(0), m_live_interval(5)
	{}

	inline void init(IN Persist& parent)
	{
		m_orm_version.id("version").table("_version");
		m_orm_version.add().property("value").field("version").pk().int_type().index();
		m_orm_version.add().property("module").field("module").pk().varchar(150).index();
		m_orm_version.add().property("timestamp").field("timestamp").date_time();
	}

	bool on_init();
	void on_live();
	void on_live_raw();
};
////////////////////////////////////////////////////////////////////////////////
bool Persist::Impl::on_init()
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
	m_master.reset(create_database(addr.type()));
	return (m_master != nullptr);
}


////////////////////////////////////////////////////////////////////////////////
void Persist::Impl::on_live_raw()
{
	// 4.connect to database server.
	if (!m_master->is_open())
	{
		eco::persist::Address& addr = m_address;
		m_master->open(addr);

		// logging persist detail info.
		char log[128] = { 0 };
		sprintf(log, "\n+[persist %s %s]\n""-open %s by %s(%s) at %s:%d\n",
			addr.type_name(), addr.name(),
			addr.database(), addr.user(), addr.password(),
			addr.host(), addr.port());
		ECO_INFO << log;
	}

	// 5.upgrade database according to it's current version.
	if (!m_state.has(state_init))
	{
		Loading load(m_load_thread_id = eco::this_thread::id());

		// get database version.
		char cond_sql[64] = { 0 };
		const char* app_name = eco::App::get()->name();
		sprintf(cond_sql, "where %s='%s' order by %s desc",
			m_orm_version.find("module")->field(), app_name,
			m_orm_version.find("value")->field());
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
void Persist::Impl::on_live()
{
	try
	{
		on_live_raw();
	}
	catch (std::exception& e)
	{
		ECO_LOG(error, m_address.name()) < "live" <= e.what();
	}
}


ECO_SHARED_IMPL(Persist);
////////////////////////////////////////////////////////////////////////////////
void Persist::set_address(IN const persist::Address& v)
{
	impl().m_address = v;
}
const persist::Address& Persist::address() const
{
	return impl().m_address;
}
void Persist::start()
{
	// 1.init class.
	if (!impl().on_init())
		return;

	// 2.run live.
	impl().on_live();

	// 3.live timer.
	impl().m_timer = eco::App::get()->timing().run_after(
		std::bind(&Persist::Impl::on_live, &impl()),
		std_chrono::seconds(impl().m_live_interval), true);
}


////////////////////////////////////////////////////////////////////////////////
void Persist::set_field(IN const char* prop, IN const char* field)
{
	PropertyMapping* map = impl().m_orm_version.find(prop);
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
		ECO_THROW("persist is unready, unload data.");
	}
	return *impl().m_master;
}
void Persist::close()
{
	impl().m_timer.cancel();
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
	if (!impl().m_state.has(Impl::state_load))
	{
		ECO_THIS_ERROR(name()) < impl().m_address.host()
			< ":" < impl().m_address.port();
		return false;
	}
	return true;
}
void Persist::set_live_interval(uint16_t sec)
{
	if (sec != 0)
		impl().m_live_interval = sec;
}


////////////////////////////////////////////////////////////////////////////////
}