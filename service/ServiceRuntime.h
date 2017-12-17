#ifndef ECO_SERVICE_DEV_SERVICE_H
#define ECO_SERVICE_DEV_SERVICE_H
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
#include <eco/ExportApi.h>
#include <eco/service/DidProtocol.h>
#include <eco/net/Address.h>


namespace eco{ ;
namespace service{ ;
namespace dev{ ;


////////////////////////////////////////////////////////////////////////////////
class ECO_API Service
{
	ECO_SHARED_API(Service);
public:
	// service name.
	void set_service_name(IN const char*);
	const char* get_service_name() const;

	// get distributed object id protocol.
	void set_did_protocol(IN const char*);
	const DidProtocol& get_did_protocol() const;

	// max object workload number by horizontal extend in future.
	void set_future_max_object_workload(const int64_t&);
	int64_t& future_max_object_workload();
	const int64_t& get_future_max_object_workload() const;

	// get current max object workload number limited by machine.
	int64_t get_max_object_workload() const;

	// get object workload number in one virtual service.
	int64_t get_virtual_service_object_workload() const;

	// virtual service number contained in each horizontal service.
	void set_horizontal_virtual_service_number(const int32_t&);
	int32_t& horizontal_virtual_service_number();
	const int32_t& get_horizontal_virtual_service_number() const;

public:
	// runtime info: service address list.
	void set_address(IN eco::net::AddressSet&);
	eco::net::AddressSet& address();
	const eco::net::AddressSet& get_address();
};


////////////////////////////////////////////////////////////////////////////////
}}}
#endif