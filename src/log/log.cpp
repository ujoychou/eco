#include <eco/log/log.hpp>
////////////////////////////////////////////////////////////////////////////////


namespace eco {
namespace log {
thread_local eco::string thread_buff;
////////////////////////////////////////////////////////////////////////////////
inline const char* name(eco::log::level value)
{
    switch (value)
    {
    case eco::log::debug: return "debug";
    case eco::log::trace: return "trace";
    case eco::log::info:  return "info";
    case eco::log::warn:  return "warn";
    case eco::log::error: return "error";
    case eco::log::fatal: return "fatal";
    }
    return " none";
}

inline const char* filename(const char* name, int nth)
{
	return "";
}


////////////////////////////////////////////////////////////////////////////////
void stream::log_args(level level, const char* file, int line, const char* title,
                      const char* format, va_list* args)
{
    // "[time] [thread] [level] <title> message (file) "
    // [20230912 15:53:35.899984] [T3928X938] [ INFO] <title> msg...(file.c:161)
	const char* name = eco::this_thread::name();
	thread_buff << '[' << date_time::stamp(date_time::std_ms).value << ']';
	if (eco::empty(name))
		thread_buff << '[' << eco::this_thread::id() << ']';
	else
		thread_buff << '[' << eco::log::name(name, 8) << ']';

	thread_buff << '[' << eco::log::name() << ']';

    // <title>
    if (title != NULL)
    {
        thread_buff << '<' << title << '>';
    }
    // message
    if (format != NULL)
    {
        int left = (int)sizeof(buf) - size;
        buf[size++] = ' ';  // add space
        size += vsnprintf(buf + size, (size_t)left, format, *args);
    }
    
    // (file)
    int left = (int)sizeof(buf) - size;
    if (left > 0 && level >= ks_warn)
    {
		file = eco::log::filename(file, 2);
        size += snprintf(buf + size, (size_t)left, " (%s:%d)", file, line);
    }
    thread_buff << '\n';
	eco::log::post(thread_buff);
}


////////////////////////////////////////////////////////////////////////////////
}}