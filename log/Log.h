#ifndef ECO_LOG_LOG_H
#define ECO_LOG_LOG_H
/*******************************************************************************
@ name
logging.

@ function
1.support logging file and console.

@ exception

@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/log/Type.h>
#include <eco/log/Core.h>
#include <eco/log/Pusher.h>


////////////////////////////////////////////////////////////////////////////////
#define EcoTrace\
	if (eco::log::trace >= eco::log::get_core().get_severity_level())\
		eco::log::FixPusher().set(__FILE__, __LINE__, eco::log::trace).stream()

#define EcoDebug\
	if (eco::log::debug >= eco::log::get_core().get_severity_level())\
		eco::log::FixPusher().set(__FILE__, __LINE__, eco::log::debug).stream()

#define EcoInfo\
	if (eco::log::info  >= eco::log::get_core().get_severity_level())\
		eco::log::FixPusher().set(__FILE__, __LINE__, eco::log::info).stream()

#define EcoWarn\
	if (eco::log::warn  >= eco::log::get_core().get_severity_level())\
		eco::log::FixPusher().set(__FILE__, __LINE__, eco::log::warn).stream()

#define EcoError\
	if (eco::log::error >= eco::log::get_core().get_severity_level())\
		eco::log::FixPusher().set(__FILE__, __LINE__, eco::log::error).stream()

#define EcoFatal\
	if (eco::log::fatal >= eco::log::get_core().get_severity_level())\
		eco::log::FixPusher().set(__FILE__, __LINE__, eco::log::fatal).stream()

#define EcoLog(sev, size)\
	if (eco::log::##sev  >= eco::log::get_core().get_severity_level())\
		eco::log::FixPusherT<size>().set(\
		__FILE__, __LINE__, eco::log::##sev).stream()

/* log message with heap memory, it has no length limit instead of FixPusher.
it's buffer implement is eco::String, so you can dedicated a reserve size.
*/
#define EcoLogStr(sev, size)\
	if (eco::log::##sev  >= eco::log::get_core().get_severity_level())\
		eco::log::Pusher(size).set(\
		__FILE__, __LINE__, eco::log::##sev).stream()

////////////////////////////////////////////////////////////////////////////////
#endif