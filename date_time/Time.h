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
#include <eco/Export.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(date_time);
////////////////////////////////////////////////////////////////////////////////
class ECO_API Time
{
public:
	// get daily total seconds.
	static uint32_t today_seconds();

    inline static uint32_t day_seconds()
	{
		return 86400;	// 60 * 60 * 24
	}

    inline static uint32_t get_minute(IN const char* time)
    {
        uint32_t h = eco::cast<uint16_t>(time, 2);
        uint32_t m = eco::cast<uint16_t>(&time[3], 2);
        return h * 60 + m;
    }

    inline static uint32_t get_minute(IN const uint32_t sec)
    {
        return sec / 60;
    }

    inline static uint32_t get_second(IN const char* time)
    {
        uint32_t s = eco::cast<uint16_t>(&time[6], 2);
        return get_minute(time) * 60 + s;
    }

	inline static uint32_t get_second(IN const std::string& time)
	{
		return get_second(time.c_str());
	}

    // get intacted time second.
    inline static uint32_t get_second_miss(IN const char* time)
    {
        uint32_t v = 0;
        size_t size = strlen(time);
        if (size > 0) v = eco::cast<uint16_t>(&time[0], 2);
        v *= 60;
        if (size > 3) v += eco::cast<uint16_t>(&time[3], 2);
        v *= 60;
        if (size > 6) v += eco::cast<uint16_t>(&time[6], 2);
        return v;
    }
    inline static uint32_t get_second_miss(IN const std::string& time)
    {
        return get_second_miss(time.c_str());
    }

public:
	inline Time(bool set = true)
	{
		m_time = set ? today_seconds() : 0;
	}

	inline operator uint32_t() const
	{
		return m_time;
	}

	inline uint32_t seconds() const
	{
		return m_time;
	}

	inline uint32_t diff_end(uint32_t end) const
	{
		return end < m_time ? end + day_seconds() - m_time : end - m_time;
	}

	inline uint32_t diff_start(uint32_t start) const
	{
		return start > m_time ? m_time + day_seconds() - start : m_time - start;
	}

	// whether current is timeout.
	static inline bool timeout(uint32_t now_t, uint32_t last_t, int timeout_sec)
	{
		int day_sec = day_seconds();
		int diff = now_t - last_t;
		if (diff <= timeout_sec - day_sec)	// when cross day
		{
			diff += day_sec;
		}
		return diff >= timeout_sec;
	}
	static inline bool timeout(uint32_t last_t, uint32_t timeout_sec)
	{
		return timeout(Time(), last_t, timeout_sec);
	}

private:
	uint32_t m_time;
};


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
