#include "Pch.h"
#include <eco/net/Address.h>
////////////////////////////////////////////////////////////////////////////////


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class Address::Impl
{
	ECO_IMPL_INIT(Address);
public:
	std::string m_name;

	// server ip address or host name.
	// exp: ip-"127.0.0.1"; hostname-"shjiyin888.xicp.net";
	std::string m_host_name;

	// server port or service name.
	// exp: port"8080"; service_name"daytime"(get standard time service)
	std::string m_service_name;
};
class AddressSet::Impl
{
	ECO_IMPL_INIT(AddressSet);
public:
	inline Impl() : m_mode(0) {}

	ServiceMode m_mode;
	std::string m_name;
	std::vector<Address> m_items;
	
};


////////////////////////////////////////////////////////////////////////////////
ECO_VALUE_IMPL(Address);
ECO_PROPERTY_STR_IMPL(Address, name);
ECO_PROPERTY_STR_IMPL(Address, host_name);
ECO_PROPERTY_STR_IMPL(Address, service_name);
ECO_SHARED_IMPL(AddressSet);
ECO_PROPERTY_SET_IMPL(AddressSet, Address);
ECO_PROPERTY_STR_IMPL(AddressSet, name);
ECO_PROPERTY_VAR_IMPL(AddressSet, ServiceMode, mode);


////////////////////////////////////////////////////////////////////////////////
Address::Address(IN const char* v)
{
	m_impl = new Impl();
	set(v);
}
void Address::set(IN const char* v)
{
	impl().m_host_name.clear();
	impl().m_service_name.clear();
	if (v != nullptr)
	{
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
}


////////////////////////////////////////////////////////////////////////////////
void Address::set(IN const char* ip, IN uint32_t port)
{
	impl().m_host_name = ip;
	impl().m_service_name = eco::Integer<uint32_t>(port).c_str();
}


////////////////////////////////////////////////////////////////////////////////
uint32_t Address::port() const
{
	return eco::cast<uint32_t>(impl().m_service_name);
}
eco::String Address::value() const
{
	eco::String temp(impl().m_host_name);
	temp << ':' << impl().m_service_name;
	return temp;
}


////////////////////////////////////////////////////////////////////////////////
bool Address::ip_format() const
{
	return port() > 0 && !impl().m_host_name.empty();
}
bool Address::empty() const
{
	return impl().m_service_name.empty() || impl().m_host_name.empty();
}


////////////////////////////////////////////////////////////////////////////////
}}