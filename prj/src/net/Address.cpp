#include "PrecHeader.h"
#include <eco/net/Address.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>



////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class Address::Impl
{
public:
	void init(IN Address&) {}

	// server ip address or host name.
	// exp: ip-"127.0.0.1"; hostname-"shjiyin888.xicp.net";
	std::string m_host_name;

	// server port or service name.
	// exp: port"8080"; service_name"daytime"(get standard time service)
	std::string m_service_name;
};
class AddressSet::Impl
{
public:
	void init(IN AddressSet&) {}

	std::vector<Address> m_items;
	ServiceMode m_service_mode;
};


////////////////////////////////////////////////////////////////////////////////
ECO_VALUE_IMPL(Address);
ECO_PROPERTY_STR_IMPL(Address, host_name);
ECO_PROPERTY_STR_IMPL(Address, service_name);
ECO_SHARED_IMPL(AddressSet);
ECO_PROPERTY_SET_IMPL(AddressSet, Address);
ECO_PROPERTY_VAL_IMPL(AddressSet, ServiceMode, service_mode);


////////////////////////////////////////////////////////////////////////////////
Address::Address(IN const char* v)
{
	m_impl = new Impl();
	reset(v);
}
void Address::reset(IN const char* v)
{
	impl().m_host_name.clear();
	impl().m_service_name.clear();
	if (v == nullptr)
	{
		return;
	}

	// format "x:y"
	std::string addr(v);
	size_t pos = addr.find(':');
	if (pos != std::string::npos)
	{
		impl().m_host_name = addr.substr(0, pos);
		pos = pos + 1;
		impl().m_service_name = addr.substr(pos, addr.size() - pos);
	}
}


////////////////////////////////////////////////////////////////////////////////
void Address::reset(IN const char* ip, IN const uint32_t port)
{
	impl().m_host_name = ip;
	impl().m_service_name = eco::Integer<uint32_t>(port).c_str();
}


////////////////////////////////////////////////////////////////////////////////
const uint32_t Address::get_port() const
{
	return eco::cast<uint32_t>(impl().m_service_name);
}


////////////////////////////////////////////////////////////////////////////////
bool Address::ip_format() const
{
	return get_port() > 0 && !impl().m_host_name.empty();
}
bool Address::empty() const
{
	return impl().m_service_name.empty() || impl().m_host_name.empty();
}


////////////////////////////////////////////////////////////////////////////////
}}