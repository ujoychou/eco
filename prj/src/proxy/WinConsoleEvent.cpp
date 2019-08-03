#include "PrecHeader.h"
#ifdef WIN32
#include <eco/proxy/WinConsoleEvent.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/App.h>


ECO_NS_BEGIN(eco);
extern App* s_app;
ECO_NS_BEGIN(win);
////////////////////////////////////////////////////////////////////////////////
BOOL OnConsole(DWORD msg)
{
	if (s_app == 0) return TRUE;

	switch (msg)
	{
	case CTRL_C_EVENT:
		return s_app->on_console(eco::cmd::ctrl_c);
	case CTRL_CLOSE_EVENT:
		return s_app->on_console(eco::cmd::ctrl_close);
	case CTRL_BREAK_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	default:
		return TRUE;
	}
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
bool ConsoleEvent::init()
{
	// register windows consle handler.
	return SetConsoleCtrlHandler((PHANDLER_ROUTINE)OnConsole, true);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(win);
ECO_NS_END(eco);
#endif