#include "PrecHeader.h"
#include <eco/persist/Address.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>



////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace persist{;


////////////////////////////////////////////////////////////////////////////////
class Address::Impl
{
public:
	void init(IN Address&)
	{
		m_port = 0;
		m_type = 0;
		m_char_set = char_set_gbk;
	}

	SourceType	m_type;
	CharSet		m_char_set;
	uint32_t	m_port;
	std::string m_name;
	std::string m_host;
	std::string m_database;
	std::string m_user;
	std::string m_password;
};
class AddressSet::Impl
{
public:
	void init(IN AddressSet&) {}
	std::vector<Address> m_items;
};


////////////////////////////////////////////////////////////////////////////////
ECO_VALUE_IMPL(Address);
ECO_PROPERTY_STR_IMPL(Address, name);
ECO_PROPERTY_STR_IMPL(Address, host);
ECO_PROPERTY_STR_IMPL(Address, user);
ECO_PROPERTY_STR_IMPL(Address, database);
ECO_PROPERTY_STR_IMPL(Address, password);
ECO_PROPERTY_VAV_IMPL(Address, uint32_t, port);
ECO_PROPERTY_VAV_IMPL(Address, SourceType, type);
ECO_PROPERTY_VAV_IMPL(Address, CharSet, char_set);
ECO_SHARED_IMPL(AddressSet);
ECO_PROPERTY_SET_IMPL(AddressSet, Address);


////////////////////////////////////////////////////////////////////////////////
void Address::set_type(const char* type)
{
	if (strcmp(type, "mysql") == 0)
		set_type(source_mysql);
	else if (strcmp(type, "sqlite") == 0)
		set_type(source_sqlite);
}
const char* Address::get_type_name() const
{
	if (impl().m_type == source_mysql)
		return "mysql";
	else if (impl().m_type == source_sqlite)
		return "sqlite";
	return "unknown";
}


////////////////////////////////////////////////////////////////////////////////
Address& Address::set(IN const char* v)
{
	impl().m_host.clear();
	impl().m_port = 0;
	if (v == nullptr)
	{
		return *this;
	}

	// format "x:y"
	std::string addr(v);
	size_t pos = addr.find(':');
	if (pos != std::string::npos)
	{
		impl().m_host = addr.substr(0, pos++);
		impl().m_port = eco::cast<uint32_t>(
			addr.substr(pos, addr.size() - pos));
	}
	return *this;
}


////////////////////////////////////////////////////////////////////////////////
}}