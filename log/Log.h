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

ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(this_thread);
ECO_API eco::log::PusherT<eco::String>& logbuf();
ECO_NS_END(this_thread);
ECO_NS_BEGIN(log);
////////////////////////////////////////////////////////////////////////////////
class Logger
{
public:
	inline eco::String& set(
		IN const char* func_name,
		IN const char* file_name,
		IN int file_line,
		IN SeverityLevel sev_level)
	{
		m_buf = &eco::this_thread::logbuf();
		return m_buf->set(eco::Bytes(func_name),
			file_name, file_line, sev_level).stream();
	}

	inline eco::String& set(
		IN const eco::Bytes& func_name,
		IN const char* file_name,
		IN int file_line,
		IN SeverityLevel sev_level)
	{
		m_buf = &eco::this_thread::logbuf();
		return m_buf->set(func_name, file_name, file_line, sev_level).stream();
	}

	inline ~Logger()
	{
		m_buf->pop();
	}

private:
	eco::log::Pusher* m_buf;
};
ECO_NS_END(log);
ECO_NS_END(eco);


////////////////////////////////////////////////////////////////////////////////
#define ECO_DEBUG ECO_LOG(debug)
#define ECO_INFO  ECO_LOG(info)
#define ECO_KEY   ECO_LOG(key)
#define ECO_WARN  ECO_LOG(warn)
#define ECO_ERROR ECO_LOG(error)
#define ECO_FATAL ECO_LOG(fatal)

/* logging stream message. 
@sev: severity_level.
@func: logging func string.
@unlimited_size: logging unlimited size.
*/
#define ECO_LOG_1(sev) ECO_LOG_2(sev, nullptr)
#define ECO_LOG_2(sev, func) \
if (eco::log::sev >= eco::log::core().severity_level())\
	eco::log::Logger().set(func, __FILE__, __LINE__, eco::log::sev)
#define ECO_LOG(...) ECO_MACRO(ECO_LOG_,__VA_ARGS__)
#define ECO_FUNC(sev) ECO_LOG(sev, __func__)


////////////////////////////////////////////////////////////////////////////////
// logging message with a severity level value "integer type".
#define ECO_LOG_SEV_1(sev) ECO_LOG_SEV_2(sev, nullptr)
#define ECO_LOG_SEV_2(sev, func)\
if (sev >= eco::log::core().severity_level())\
	eco::log::Logger().set(func, __FILE__, __LINE__, sev)
#define ECO_LOG_SEV(...) ECO_MACRO(ECO_LOG_SEV_,__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////
#endif