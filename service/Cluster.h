#ifndef ECO_SERVICE_CLUSTER_H
#define ECO_SERVICE_CLUSTER_H
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
#include <eco/service/DidProtocol.h>
#include <eco/service/Service.h>
#include <eco/service/Machine.h>
#include <eco/service/Deployment.h>


namespace eco{ ;
namespace service{ ;


////////////////////////////////////////////////////////////////////////////////
class ECO_API Cluster
{
	ECO_IMPL_API();
	ECO_SINGLETON(Cluster);
public:
	// login dops centre.
	bool login();

	// create a new service.
	Service add_service();

	// find service by name.
	Service find_service(IN const char* service_name);

	// create a new machine.
	Service add_machine();

	// find machine by name.
	Service find_machine(IN const char* service_name);

	// deploy service on the machine.
	Deployment add_deploy();

	// find deployment set of machine.
	DeploymentSet find_machine_deploy(IN const char* mac_ip);

	// find deployment set of service.
	DeploymentSet find_service_deploy(IN const char* service_name);
};


ECO_API Cluster& get_cluster();
////////////////////////////////////////////////////////////////////////////////
}}
#endif