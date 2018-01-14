#include "PrecHeader.h"
#include <eco/persist/Persist.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/meta/Meta.h>
#include <eco/persist/Database.h>
#include <eco/thread/State.h>


namespace eco{;


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
	PersistHandler*			m_handler;
	eco::Database::ptr		m_database;
	std::vector<Upgrade>	m_upgrade_seq;
	eco::ObjectMapping		m_version_orm;
	eco::atomic::State		m_state;
	persist::AddressSet		m_address_set;

	void on_live();
	bool on_upgrade(IN eco::persist::Version& v);
};

////////////////////////////////////////////////////////////////////////////////
bool Persist::on_born()
{
	if (impl().m_handler != nullptr)
	{
		// 1.初始化映射关系
		impl().m_handler->on_mapping();

		// 2.初始化数据库升级
		impl().m_handler->on_upgrade(*this);
		std::sort(impl().m_upgrade_seq.begin(), impl().m_upgrade_seq.end());
	}

	// 3.初始化数据库
	eco::persist::AddressSet addr_set;
	//impl().m_database.reset(new eco::MySql());
	return true;
}


////////////////////////////////////////////////////////////////////////////////
void Persist::Impl::on_live()
{
	// 2.连接数据库
	if (!m_database->is_open())
	{
		m_database->open(get_app().get_sys_db_address());
	}

	// 3.升级数据库（根据当前版本升级数据库）
	if (!m_state.is_ok())
	{
		eco::persist::Version version;
		m_database->create_table(m_version_orm);
		m_database->select<eco::persist::VersionMeta>(version, m_version_orm);
		if (!on_upgrade(version))
		{
			return;
		}
		m_state.ok();
	}

	try
	{
		// 日志
		std::string db = get_app().get_config().at("basic/db");
		std::string user = get_app().get_config().at("basic/user");
		std::string pwd = get_app().get_config().at("basic/password");
		std::string addr = get_app().get_config().at("basic/addr");
		eco::net::Address addr_obj(addr.c_str());

		// 日志
		EcoInfo << "database " << addr_obj.get_host_name()
			<< " " << addr_obj.get_port() << " " << db << " " << user;

		// 连接服务器
		m_master->open(
			addr_obj.get_host_name(),
			addr_obj.get_port(),
			db.c_str(), user.c_str(), pwd.c_str());

		//获取数据库版本号
		m_master->create_table(version);
		eco::Record rd;
		std::string sql = ("select max(version) from ");
		sql += get_orm().GetVerDb().get_table();
		m_master->select(rd, sql.c_str());
		uint32_t version = eco::cast<uint32_t>(rd[0]);
	}
	catch (std::exception& e)
	{
	}
}


////////////////////////////////////////////////////////////////////////////////
bool Persist::Impl::on_upgrade(IN eco::persist::Version& v)
{
	for (auto it = m_upgrade_seq.begin(); it != m_upgrade_seq.end(); ++it)
	{
		if (it->m_version > v.m_value)
		{
			if (!it->m_func(*m_database))
			{
				return false;
			}
			m_database->save<eco::persist::VersionMeta>(
				v.value(it->m_version), m_version_orm);
		}
	}
	return true;
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

void Persist::register_upgrade(IN const uint32_t ver, IN UpgradeFunc func)
{
	impl().m_upgrade_seq.push_back(Upgrade(ver, func));
}


////////////////////////////////////////////////////////////////////////////////
}