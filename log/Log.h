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
#define ECO_DEBUG ECO_LOG(debug)
#define ECO_INFO  ECO_LOG(info)
#define ECO_WARN  ECO_LOG(warn)
#define ECO_ERROR ECO_LOG(error)
#define ECO_FATAL ECO_LOG(fatal)

/* logging stream message. 
@sev: severity_level.
@func: logging func string.
@unlimited_size: logging unlimited size.
*/
#define ECO_LOG_1(sev) ECO_LOG_2(sev, nullptr)
#define ECO_LOG_2(sev, func)\
	if (eco::log::##sev >= eco::log::get_core().get_severity_level())\
		eco::log::Pusher<>().set(\
		func, __FILE__, __LINE__, eco::log::##sev).stream()
#define ECO_LOG(...) ECO_MACRO(ECO_LOG_,__VA_ARGS__)

#define ECO_LOGX_1(sev) ECO_LOGX_2(sev, nullptr)
#define ECO_LOGX_2(sev, func)\
	if (eco::log::##sev >= eco::log::get_core().get_severity_level())\
		eco::log::PusherX(256).set(\
		func, __FILE__, __LINE__, eco::log::##sev).stream()
#define ECO_LOGX(...) ECO_MACRO(ECO_LOGX_,__VA_ARGS__)

/* logging stream message.
@sev: severity_level.
@unlimited_size,: logging unlimited size.
*/
#define ECO_FUNC(sev) ECO_LOG(sev, __func__)
#define ECO_FUNCX(sev) ECO_LOGX(sev, __func__)

// log message with heap memory, it has no length limit instead of FixPusher.
// it's buffer implement is eco::String, so you can dedicated a reserve size.
////////////////////////////////////////////////////////////////////////////////
#endif