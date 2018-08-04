#ifndef ECO_WIN_DUMP_H
#define ECO_WIN_DUMP_H
#ifdef WIN32
////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
@ name
os proxy, as a adapter for all operation system.

@ function
as


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-01-15.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Object.h>
#include <string>
#include "windows.h"
#include <DbgHelp.h>


////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(win);


////////////////////////////////////////////////////////////////////////////////
class ECO_API Dump
{
public:
	// init dump info so that it can produce dump file when app crash.
	static void init(IN bool auto_restart = false);
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(win);
ECO_NS_END(eco);
#endif
#endif