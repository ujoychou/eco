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
template<typename Stream>
class PusherT
{
public:
	typedef Stream Stream;

	inline PusherT();
	~PusherT();

	/*@ logging collector. domain has 'logging', 'monitor', 'report'
	*/
	PusherT& set(
		IN const char* file_name,
		IN int file_line,
		IN SeverityLevel sev_level);

	/*@ log stream.*/
	inline Stream& stream()
	{
		return m_stream;
	}

protected:
	Stream m_stream;
	SeverityLevel m_severity;
	uint32_t	m_file_line;
	const char* m_file_name;
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
{
	typedef PusherT<eco::StreamT<FixBuffer<size> > > Super;
public:
	inline FixPusherT()
	{}
};
typedef FixPusherT<text_size> FixPusher;
typedef FixPusher::Stream LogStream;

////////////////////////////////////////////////////////////////////////////////
}}
#include <eco/log/Pusher.inl>
#endif