#pragma once
/*******************************************************************************
@ name
dll entry

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/rtti/api.hpp>


eco_namespace(eco);
eco_namespace(rx);
////////////////////////////////////////////////////////////////////////////////
// app message send to erx dll.
enum
{
	message_init		= 0x0001,
	message_cmd			= 0x0002,
	message_load		= 0x0003,
	message_exit		= 0x0004,
};
typedef int message;


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(rx);
eco_namespace_end(eco);