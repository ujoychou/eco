#include "PrecHeader.h"
#ifdef WIN32
#include <eco/sys/WinConsoleEvent.h>
////////////////////////////////////////////////////////////////////////////////
#include "../Impl.h"


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(win);
////////////////////////////////////////////////////////////////////////////////
BOOL OnConsole(DWORD msg)
{
	if (eco::App::get() == nullptr)
	{
		return TRUE;
	}

	switch (msg)
	{
	case CTRL_C_EVENT:
		return AppInner::on_console(*eco::App::get(), eco::cmd::ctrl_c);
	case CTRL_CLOSE_EVENT:
		return AppInner::on_console(*eco::App::get(), eco::cmd::ctrl_close);
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
	return SetConsoleCtrlHandler((PHANDLER_ROUTINE)OnConsole, true) == TRUE;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(win);
ECO_NS_END(eco);
#endif