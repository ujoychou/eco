#ifndef ECO_WIN_CONSOLE_EVENT_H
#define ECO_WIN_CONSOLE_EVENT_H
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
#include <eco/Export.h>
#include "windows.h"


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(win);
////////////////////////////////////////////////////////////////////////////////
class ECO_API ConsoleEvent
{
public:
	static bool init();
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(win);
ECO_NS_END(eco);
#endif
#endif