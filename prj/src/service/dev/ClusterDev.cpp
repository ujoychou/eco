#include "PrecHeader.h"
#include <eco/service/dev/Cluster.h>
#include <eco/service/dev/Ecode.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/thread/Map.h>


namespace eco { ;
namespace service { ;
namespace dev { ;


////////////////////////////////////////////////////////////////////////////////
Cluster& get_cluster()
{
	return eco::Singleton<Cluster>::instance();
}


////////////////////////////////////////////////////////////////////////////////
class Cluster::Impl
{
public:
	void init(IN Cluster&) {}

	std::unordered_map<std::string, eco::net::TcpClient> m_router_map;
	std::unordered_map<std::string, eco::net::TcpClient> m_service_map;
	std::unordered_map<std::string, eco::net::AddressSet> m_router_service_map;
	eco::Mutex m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
ECO_TYPE_IMPL(Cluster);


////////////////////////////////////////////////////////////////////////////////
/*const eco::service::Service Cluster::register_service(
	IN const char* service_name,
	IN const char* router_name,
	IN const int port)
{
	eco::service::Service service;
	return service;
}*/


////////////////////////////////////////////////////////////////////////////////
void Cluster::async_init_service(
	OUT eco::net::TcpClient& client,
	IN const char* service_name,
	IN eco::net::AddressSet& service_addr)
{
	eco::Mutex::ScopeLock lock(impl().m_mutex);
	auto it = impl().m_service_map.find(service_name);
	if (it != impl().m_service_map.end())
	{
		//it->second.async_connect(service_name, service_addr);
		return ;
	}
}


////////////////////////////////////////////////////////////////////////////////
void async_init_service(
	OUT eco::net::TcpClient& tcp_client,
	IN const char* service_name,
	IN eco::net::AddressSet& service_addr)
{
}


////////////////////////////////////////////////////////////////////////////////
void get_service(
	IN const char* service_name)
{
}


////////////////////////////////////////////////////////////////////////////////
}}}