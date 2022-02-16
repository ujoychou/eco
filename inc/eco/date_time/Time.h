#ifndef ECO_TIME_H
#define ECO_TIME_H
/*******************************************************************************
@ name
convert types.

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2018-06-03.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, muduo.chenshuo, reserved all right.

*******************************************************************************/
#include <eco/rx/RxExport.h>
#include <eco/std/chrono.h>
#include <eco/Error.h>
#include <ctime>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(date_time);
////////////////////////////////////////////////////////////////////////////////
inline uint32_t day_seconds()
{
	return 86400;	// 60 * 60 * 24
}
inline uint32_t get_minute(IN const char* time)
{
	uint32_t h = eco::cast<uint16_t>(&time[0], 2);
	uint32_t m = eco::cast<uint16_t>(&time[3], 2);
	return h * 60 + m;
}
inline uint32_t get_minute(IN const uint32_t sec)
{
	return sec / 60;
}
inline uint32_t get_second(IN const char* time)
{
	uint32_t v = 0;
	v = eco::cast<uint16_t>(&time[0], 2);  v *= 60;
	v += eco::cast<uint16_t>(&time[3], 2); v *= 60;
	return v += eco::cast<uint16_t>(&time[6], 2);
}
inline uint32_t get_second(IN const std::string& time)
{
	return get_second(time.c_str());
}
inline static uint32_t get_second_miss(IN const char* time)
{
	uint32_t v = 0;
	size_t size = strlen(time);
	if (size >= 2) v = eco::cast<uint16_t>(&time[0], 2);
	v *= 60;
	if (size >= 5) v += eco::cast<uint16_t>(&time[3], 2);
	v *= 60;
	if (size >= 8) v += eco::cast<uint16_t>(&time[6], 2);
	return v;
}
inline static uint32_t get_second_miss(IN const std::string& time)
{
	return get_second_miss(time.c_str());
}
inline bool timeout(uint32_t now_t, uint32_t last_t, int timeout_sec)
{
	// whether current is timeout.
	int day_sec = day_seconds();
	int diff = now_t - last_t;
	if (diff <= timeout_sec - day_sec)	// when cross day
	{
		diff += day_sec;
	}
	return diff >= timeout_sec;
}


////////////////////////////////////////////////////////////////////////////////
inline uint32_t now_time()
{
	std_chrono::system_clock::time_point tp = std_chrono::system_clock::now();
	std::time_t tt = std_chrono::system_clock::to_time_t(tp);
	struct std::tm* t = std::localtime(&tt);
	return t->tm_hour * 3600 + t->tm_min * 60 + t->tm_sec;
}


////////////////////////////////////////////////////////////////////////////////
inline std_chrono::system_clock::time_point make_time_point(
	int year, int mon, int day, int hour, int min, int sec)
{
	struct std::tm t;
	t.tm_sec = sec;
	t.tm_min = min;
	t.tm_hour = hour;
	t.tm_mday = day;
	t.tm_mon = mon - 1;
	t.tm_year = year - 1900;
	t.tm_isdst = -1;
	std::time_t tt = std::mktime(&t);
	if (tt == -1)
	{
		ECO_THROW("no valid system time.");
	}
	return std_chrono::system_clock::from_time_t(tt);
}
inline std_chrono::system_clock::time_point make_time_point(const char* tp)
{
	size_t len = strlen(tp);
	int year = 0, mon = 0, day = 0;
	int hour = 0, min = 0, sec = 0;
	if (len == 17)			// 20201025 15:30:00
	{
		year = eco::cast<int>(&tp[0], 4);
		mon = eco::cast<int>(&tp[4],  2);
		day = eco::cast<int>(&tp[6],  2);
		hour = eco::cast<int>(&tp[9], 2);
		min = eco::cast<int>(&tp[12], 2);
		sec = eco::cast<int>(&tp[15], 2);
	}
	else if (len == 19)		// 2020-10-25 15:30:00
	{
		year = eco::cast<int>(&tp[0], 4);
		mon = eco::cast<int>(&tp[5],  2);
		day = eco::cast<int>(&tp[8],  2);
		hour = eco::cast<int>(&tp[11], 2);
		min = eco::cast<int>(&tp[14], 2);
		sec = eco::cast<int>(&tp[17], 2);
	}
	return make_time_point(year, mon, day, hour, min, sec);
}


////////////////////////////////////////////////////////////////////////////////
class Time
{
public:
	inline Time(bool set = true)
	{
		m_time = set ? now_time() : 0;
	}

	inline Time(const uint32_t v) : m_time(v)
	{}

	inline Time(const char* now_t)
	{
		m_time = get_second(now_t);
	}

	inline Time(const struct tm& t)
	{
		m_time = t.tm_hour * 3600 + t.tm_min * 60 + t.tm_sec;
	}

	inline void set_value(const uint32_t v)
	{
		m_time = v;
	}

	inline operator uint32_t() const
	{
		return m_time;
	}

	inline uint32_t seconds() const
	{
		return (uint32_t)m_time;
	}

	inline uint32_t diff_end(uint32_t end) const
	{
		return end < m_time ? end + day_seconds() - m_time : end - m_time;
	}

	inline uint32_t diff_start(uint32_t start) const
	{
		return start > m_time ?
			m_time + day_seconds() - start : m_time - start;
	}

private:
	uint32_t m_time;
};
inline bool timeout(uint32_t last_t, uint32_t timeout_sec)
{
	return timeout(Time(), last_t, timeout_sec);
}


////////////////////////////////////////////////////////////////////////////////
class SizeFlow
{
public:
	// flow
	uint64_t m_size;
	uint32_t m_size_delta;
	uint32_t m_size_flow;
	// time
	uint16_t m_interval;
	uint16_t m_diff;
	Time m_curr;
	Time m_last;

public:
	inline SizeFlow(IN uint32_t interval = 5)
	{
		memset(this, 0, sizeof(*this));
		m_interval = interval;
	}

	inline bool count(IN uint32_t size = 1)
	{
		m_size += size;
		m_size_delta += size;
		m_curr = Time();
		m_diff = m_curr.diff_start(m_last);
		if (m_interval <= m_diff)
		{
			m_size_flow = m_size_delta / m_diff;
			m_size_delta = 0;
			m_last = m_curr;
			return true;
		}
		return false;
	}
};


////////////////////////////////////////////////////////////////////////////////
class ByteFlow : public SizeFlow
{
public:
	uint64_t m_byte;
	uint32_t m_byte_delta;
	uint32_t m_byte_flow;

public:
	inline ByteFlow(IN uint32_t interval = 5) : SizeFlow(interval)
	{
		m_byte = 0;
		m_byte_flow = 0;
		m_byte_delta = 0;
	}

	inline uint32_t kbyte() const
	{
		return (uint32_t)(m_byte / 1024);
	}
	inline uint32_t mbyte() const
	{
		return (uint32_t)(m_byte / (1024 * 1024));
	}

	inline uint32_t kbyte_flow() const
	{
		return m_byte_flow / 1024;
	}
	inline uint32_t mbyte_flow() const
	{
		return m_byte_flow / (1024 * 1024);
	}
	
	inline bool count(IN uint32_t byte, IN uint32_t size = 1)
	{
		m_byte += byte;
		m_byte_delta += byte;
		if (SizeFlow::count(size))
		{
			m_byte_flow = m_byte_delta / m_diff;
			m_byte_delta = 0;
			return true;
		}
		return false;
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(date_time);
ECO_NS_END(eco);
#endif
