#ifndef ECO_SERVICE_IMPL_H
#define ECO_SERVICE_IMPL_H
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
#include <eco/service/Service.h>



namespace eco{ ;
namespace service{ ;


////////////////////////////////////////////////////////////////////////////////
class Service::Impl
{
public:
	void init(IN Service&) {}
};

////////////////////////////////////////////////////////////////////////////////
}}
#endif