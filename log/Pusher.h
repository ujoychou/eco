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
#include <eco/filesystem/SourceFile.h>



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
template<typename Stream>
class PusherT
{
public:
	typedef Stream Stream;

	inline PusherT() : m_severity(eco::log::none)
	{}
	~PusherT();

	/*@ logging collector. domain has 'logging', 'monitor', 'report'
	*/
	PusherT& set(
		IN const char* file_name,
		IN int file_line,
		IN SeverityLevel sev_level,
		IN const char* domain = nullptr);

	/*@ log stream.*/
	inline Stream& stream()
	{
		return m_stream;
	}

protected:
	Stream m_stream;
	SeverityLevel m_severity;
	std::auto_ptr<eco::filesystem::SourceFile> m_src;
};


////////////////////////////////////////////////////////////////////////////////
class Pusher : public PusherT<eco::Stream>
{
public:
	inline Pusher(IN const uint32_t size)
	{
		m_stream.buffer().reserve(size);
	}
};


////////////////////////////////////////////////////////////////////////////////
template<uint32_t size>
class FixPusherT : public PusherT<eco::StreamT<FixBuffer<size> > >
{};
typedef FixPusherT<log_text_size> FixPusher;
typedef FixPusher::Stream LogStream;

////////////////////////////////////////////////////////////////////////////////
}}
#include <eco/log/Pusher.inl>
#endif