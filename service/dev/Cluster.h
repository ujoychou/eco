#ifndef ECO_SERVICE_DEV_CLUSTER_H
#define ECO_SERVICE_DEV_CLUSTER_H
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2017-01-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Object.h>
#include <eco/meta/Timestamp.h>
#include <eco/net/Address.h>
#include <eco/net/TcpClient.h>
#include <eco/service/Service.h>


namespace eco{ ;
namespace service{ ;
namespace dev{ ;


////////////////////////////////////////////////////////////////////////////////
class ECO_API Cluster
{
	ECO_IMPL_API();
	ECO_SINGLETON_UNINIT(Cluster);
//////////////////////////////////////////////////////////////////////// ROUTER
public:
	// publish service address.
	virtual void on_pub_service_address(
		IN const char* service_name,
		IN const eco::net::Address& addr,
		IN const eco::meta::Timestamp& ts)
	{}

	// publish service horizontal address.
	virtual void on_pub_horizontal_address(
		IN const char* service_name,
		IN const uint32_t horizontal_id,
		IN const eco::net::Address& addr,
		IN const eco::meta::Timestamp& ts)
	{}

public:
	// init router address.
	void init_router(
		IN const char* router_name,
		IN eco::net::AddressSet& router_addrset);

	// get router service.
	/*const eco::service::Service register_service(
		IN const char* service_name,
		IN const char* router_name,
		IN const int port);

	// get router service who has no status.
	const eco::service::Service get_service(
		IN const char* router_name,
		IN const char* service_name);*/

	// call a router service who has data status with horizontal split.
	eco::net::TcpClient get_horizontal(
		IN const char* ops_name,
		IN const char* service_name,
		IN const uint64_t object_did);

	// get service address.
	eco::net::AddressSet get_service_address(
		IN const char* router_name,
		IN const char* service_name);

	// sub service address.
	void sub_service_address(
		IN const char* router_name,
		IN const char* service_name);

	// sub horizontal address.
	eco::net::AddressSet sub_horizontal_address(
		IN const char* router_name,
		IN const char* service_name,
		IN const uint32_t horizontal_id);

public:
	// init service consumer.
	void async_init_service(
		IN eco::net::TcpClient& client,
		IN const char* service_name,
		IN eco::net::AddressSet& service_addr);

	// get service consumer.
	eco::net::TcpClient get_service(
		IN const char* service_name);

	// get service address.
	eco::net::AddressSet get_service_address(
		IN const char* service_name);
};


ECO_API Cluster& get_cluster();
////////////////////////////////////////////////////////////////////////////////
}}}
#endif