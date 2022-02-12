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
#include <eco/log/Core.h>
#include <eco/thread/Thread.h>
#include <eco/date_time/DateTime.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(log);
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
template<typename stream_t>
class PusherT
{
public:
	inline PusherT()
		: m_severity(none)
		, m_file_line(0)
		, m_file_name(nullptr)
	{}

	inline ~PusherT()
	{
		pop();
	}

	// post message to server or queue.
	inline void pop()
	{
		if (m_severity < Core::get().severity_level()) return;

		const char* t_name = eco::this_thread::name();
		if (m_severity >= eco::log::warn)
		{
			// logging thread name.
			if (!eco::empty(t_name))
				m_stream << " &" << t_name;
			// logging source file info.
			if (m_file_name != nullptr)
				m_stream << " @" << m_file_name << '.' << m_file_line;
		}

		// turn line '\n' must input into the string.
		m_stream.force_append(1, '\n');
		Core::get().append(m_stream.bytes(), m_severity);
	}

	/*@ logging collector. domain has 'logging', 'monitor', 'report'
	*/
	inline PusherT& set(
		IN const eco::Bytes& func_name,
		IN const char* file_name,
		IN int file_line,
		IN SeverityLevel sev_level)
	{
		assert(sev_level >= Core::get().severity_level());
		m_severity = sev_level;

		// info< logging no need to save source file info.
		if (sev_level >= eco::log::warn)
		{
			uint32_t pos = -1;
			m_file_name = (
				(pos = find_last(file_name, '/'))  != uint32_t(-1) ||
				(pos = find_last(file_name, '\\')) != uint32_t(-1))
				? &file_name[pos + 1] : file_name;
			m_file_line = file_line;
		}
		else
		{
			m_file_line = 0;
			m_file_name = 0;
		}

		// default domain is empty string.
		eco::date_time::Timestamp now(eco::date_time::fmt_std_m);
		m_stream << now.value() <= eco::this_thread::sid()
			<= Severity::get_display(sev_level) < ' ';
		if (!func_name.null()) m_stream << eco::group(func_name) < ' ';
		return *this;
	}

	/*@ log stream.*/
	inline stream_t& stream()
	{
		return m_stream;
	}

protected:
	stream_t m_stream;
	SeverityLevel m_severity;
	uint32_t	m_file_line;
	const char* m_file_name;
};
typedef eco::log::PusherT<eco::String> Pusher;


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(log);
ECO_NS_END(eco);
#endif