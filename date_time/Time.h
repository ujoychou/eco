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

	static inline uint32_t day_seconds()
	{
		return 3600 * 24;	// 60 * 60 * 24
	}

public:
	inline Time(bool set = true)
	{
		if (set) m_time = today_seconds();
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
	inline SizeFlow(IN uint32_t interval = 5) : m_curr(false), m_last(false)
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
	{}

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
