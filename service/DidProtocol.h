#ifndef ECO_SERVICE_DID_PROTOCOL_H
#define ECO_SERVICE_DID_PROTOCOL_H
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
class DidProtocol
{
public:
	// protocol identity.
	const char* identity() const;

	// distributed horizontal virtual service size.
	int32_t get_horizontal_virtual_size() const;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif