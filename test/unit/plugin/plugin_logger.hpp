#pragma once
/*******************************************************************************
@ name


@ function


@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]


--------------------------------------------------------------------------------
* copyright(c) 2025 - 2025, ujoy, reserved all right.

*******************************************************************************/
#include <eco/prec.hpp>
#include <eco/log/message.hpp>


eco_namespace(eco);
eco_namespace(log);
////////////////////////////////////////////////////////////////////////////////
class logger
{
public:
	virtual ~logger() {};
	virtual void on_entry_format(message& message, int on) = 0;
    virtual void on_entry_output(message& message) = 0;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(log);
eco_namespace_end(eco);