#ifndef ECO_SERVICE_SERVICE_H
#define ECO_SERVICE_SERVICE_H
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


namespace eco{ ;
namespace service{ ;


////////////////////////////////////////////////////////////////////////////////
class ECO_API Service
{
	ECO_SHARED_API(Service);
public:
	// process name.
	void set_process_name(IN const char*);
	const char* get_process_name() const;

	// service name.
	void set_service_name(IN const char*);
	const char* get_service_name() const;

	// object name.
	void set_object_name(IN const char*);
	const char* get_object_name() const;

	// get distributed object id protocol.
	void set_did_protocol(IN const char*);
	const char* get_did_protocol() const;

	// get horizontal id of object did.
	uint32_t get_horizontal_id(
		IN const uint64_t object_did) const;

	// max object workload number by horizontal extend in future.
	void set_max_future_object_workload(int64_t);
	int64_t get_max_future_object_workload() const;

	// get current max object workload number limited by machine.
	int64_t get_max_limited_object_workload() const;

	// get object workload number in one virtual service.
	int32_t get_virtual_service_object_workload() const;

	// virtual service number contained in each horizontal service.
	void set_horizontal_virtual_service_number(int32_t);
	int32_t get_horizontal_virtual_service_number() const;

	// save data.
	void save();

	// remove service from dops centre system.
	void remove();
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif