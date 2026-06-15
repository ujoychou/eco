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
#include <eco/Object.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(datetime);
int now(int format);
ECO_NS_END(datetime);
class String;
ECO_NS_BEGIN(this_thread);
extern eco::String& logbuf();
ECO_NS_END(this_thread);





////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(log);
////////////////////////////////////////////////////////////////////////////////

enum
{
	none	= 0,
    l1      = 1,
    l2      = 2,
    l3      = 3,
    l4      = 4,
    l5      = 5,
    l6      = 6,
    l7      = 7,
    l8      = 8,
    l9      = 9,
    debug	= 10,
	trace	= 11,
	info	= 12,
	warn	= 13,
	error	= 14,
	fatal	= 15,
};
typedef int Level;

enum 
{
	// log message persist in file
	file	= 0x01,
	// log message display on console
	console	= 0x02,
	// log message send to monitor server
	monitor	= 0x04,
};
typedef int Notify;


////////////////////////////////////////////////////////////////////////////////
class Logger
{
    ECO_SINGLETON(Logger);
public:
};

class Log
{
    ECO_STREAM_DELARE();
public:
    // "[time] [thread] [level] [title] message ..."
    Log(Level level, const char* prefix, const char* file, int line, 
        const char* format, ...)
    {
        // 1: "[time] [thread] [level] [model] "
        eco::String& stream = this_thread::logbuf();
        stream << datetime::Timestamp::now(datatime::std_m);
        const char* tname = this_thread::name();
        if (!eco::empty(tname))
            stream >> eco::Square<char*>(tname, 5);
        else
            stream >> this_thread::id();
        stream >> display(level);
        
        // format2: "[title] message ..."
        if (prefix != 0) { stream >> SQ(prefix); }
        va_list args;
	    va_start(args, prefix);
        stream.format(args);
        va_end(args);

        // format3: "(file:line) \n"
        if (level >= eco::log::warn)
		{
			stream << " (" << file << ':' << line << " )";
		}
        stream.force_append(1, '\n');
    }

    ~Log()
    {
        post();
    }

private:
	void post()
	{
		Logger::get().post(*this);
	}
};



ECO_NS_END(log);
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);


////////////////////////////////////////////////////////////////////////////////
// logging message with title, setting a "integer type" log level.
#define ECO_MSG_LEVEL(level, title, ...) \
    level > eco::log::get().level() ? 0 : \
    eco::log::Log(level, __FILE__, __LINE__, title, ...)

// logging message with title.
#define ECO_MSG(level, title, ...) \
    eco::log::level > eco::log::get().level() ? 0 : \
    eco::log::Log(eco::log::level, title, __FILE__, __LINE__, ...)

// logging message without title.
#define ECO_LOG(level, ...) ECO_MSG(level, 0, ...)
#define ECO_TRACE(...) ECO_LOG(trace, __VA_ARGS__)
#define ECO_DEBUG(...) ECO_LOG(debug, __VA_ARGS__)
#define ECO_INFO(...)  ECO_LOG(info, __VA_ARGS__)
#define ECO_WARN(...)  ECO_LOG(warn, __VA_ARGS__)
#define ECO_ERROR(...) ECO_LOG(error, __VA_ARGS__)
#define ECO_FATAL(...) ECO_LOG(fatal, __VA_ARGS__)

// logging message with function title: using when debug a function.
#define ECO_FUNC(level, ...) ECO_MSG(level, __func__, __VA_ARGS__)
////////////////////////////////////////////////////////////////////////////////














void exception_test()
{
    
}


#endif
