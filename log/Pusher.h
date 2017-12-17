#ifndef ECO_LOG_PUSHER_H
#define ECO_LOG_PUSHER_H
/*******************************************************************************
@ name
logger.

@ function
1.collect log info from user thread.
2.format log info.

@ exception

@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <eco/log/Type.h>


namespace eco{;
namespace log{;


////////////////////////////////////////////////////////////////////////////////
class ECO_API Severity
{
public:
	static SeverityLevel get_level(
		IN const char* sev_name);

	static const char* get_name(
		IN const SeverityLevel sev_level);

	static const char* get_display(
		IN const SeverityLevel sev_level);
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Pusher
{
	ECO_SHARED_API(Pusher);
public:
	/*@ logging collector. domain has 'logging', 'monitor', 'report'
	*/
	Pusher& set(
		IN const char* file_name,
		IN int file_line,
		IN SeverityLevel sev_level,
		IN const char* domain = nullptr);

	/*@ log stream.*/
	LogStream& stream();
};
}}
#endif