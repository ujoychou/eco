#ifndef ECO_DATE_TIME_H
#define ECO_DATE_TIME_H
/*******************************************************************************
@ name
Now.

@ function


@ exception

@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-01-15.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/String.h>
#include <eco/date_time/Date.h>
#include <eco/date_time/Time.h>


namespace eco{;
namespace date_time{;
////////////////////////////////////////////////////////////////////////////////
class DateTime
{
public:
	inline DateTime(Date& date, uint32_t time)
		: m_date(date), m_time(time)
	{}

    inline DateTime(const std::string& time)
    {
        m_time = get_second(&time[9]);
        m_date = eco::date_time::Date(time);
    }

	inline uint32_t daily_seconds() const
	{
		return m_time;
	}

	inline int days() const
	{
		return m_date.days();
	}

	inline const Date& date() const
	{
		return m_date;
	}

	inline const Time& time() const
	{
		return m_time;
	}

	inline uint64_t total_seconds() const
	{
		return m_date.days() * day_seconds() + m_time;
	}

private:
	Time m_time;
	Date m_date;
};


////////////////////////////////////////////////////////////////////////////////
inline DateTime now(int delay_millsec = 0)
{
	auto tp = std_chrono::system_clock::now();
	tp += std_chrono::milliseconds(delay_millsec);
	std::time_t tt = std_chrono::system_clock::to_time_t(tp);
	struct std::tm* tm = std::localtime(&tt);
	return DateTime(Date(tm), Time(tm));
}





////////////////////////////////////////////////////////////////////////////////
inline void format_time(char* buf, size_t siz, Fmt fmt, std::tm* t, int mills)
{
	int y = t->tm_year + 1900;
	int m = t->tm_mon + 1;
	switch (fmt)
	{
	case eco::date_time::fmt_iso:
		snprintf(buf, siz, "%4d%02d%02d%02d%02d%02d",
			y, m, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
		break;
	case eco::date_time::fmt_iso_m:
		snprintf(buf, siz, "%4d%02d%02d%02d%02d%02d%06d",
			y, m, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, mills);
		break;
	case eco::date_time::fmt_isot:
		snprintf(buf, siz, "%4d%02d%02d %02d:%02d:%02d",
			y, m, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
		break;
	case eco::date_time::fmt_isot_m:
		snprintf(buf, siz, "%4d%02d%02d %02d:%02d:%02d.%06d",
			y, m, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, mills);
		break;
	case eco::date_time::fmt_std:
		snprintf(buf, siz, "%4d-%02d-%02d %02d:%02d:%02d",
			y, m, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
		break;
	case eco::date_time::fmt_std_m:
	default:
		snprintf(buf, siz, "%4d-%02d-%02d %02d:%02d:%02d.%06d",
			y, m, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, mills);
		break;
	}
}


////////////////////////////////////////////////////////////////////////////////
class Timestamp
{
public:
	inline explicit Timestamp(IN Fmt fmt = fmt_isot) : m_format(fmt), m_mills(0)
	{
		m_buffer[0] = 0;
		auto tp = std_chrono::system_clock::now();
		std::time_t tt = std_chrono::system_clock::to_time_t(tp);
		struct std::tm* tm = std::localtime(&tt);
		m_mills = std_chrono::duration_cast<std_chrono::milliseconds>(
			tp.time_since_epoch()).count();
		int mills = m_mills % 1000000;
		format_time(m_buffer, sizeof(m_buffer), m_format, tm, mills);
	}

	// get "time part" of the time string.
	inline const char* get_time() const
	{
		switch (m_format)
		{
		case eco::date_time::fmt_iso:
		case eco::date_time::fmt_iso_m:
			return &m_buffer[8];
		case eco::date_time::fmt_isot:
		case eco::date_time::fmt_isot_m:
			return &m_buffer[9];
		case eco::date_time::fmt_std:
		case eco::date_time::fmt_std_m:
		default:
			break;
		}
		return &m_buffer[11];
	}

	inline operator const char*() const
	{
		return m_buffer;
	}

	inline const char* value() const
	{
		return m_buffer;
	}

	inline std::string get_date() const
	{
		std::string val;
		switch (m_format)
		{
		case eco::date_time::fmt_iso:
		case eco::date_time::fmt_iso_m:
		case eco::date_time::fmt_isot:
		case eco::date_time::fmt_isot_m:
			return val.assign(m_buffer, 8);
		case eco::date_time::fmt_std:
		case eco::date_time::fmt_std_m:
		default:
			break;
		}
		return val.assign(m_buffer, 10);
	}

	inline bool operator==(IN const Timestamp& ts) const
	{
		return ts.m_format == m_format && strcmp(m_buffer, ts.m_buffer) == 0;
	}
	inline bool operator!=(IN const Timestamp& ts) const
	{
		return !(operator==(ts));
	}
	inline bool operator<(IN const Timestamp& ts) const
	{
		return ts.m_format == m_format && strcmp(m_buffer, ts.m_buffer) < 0;
	}

	// total seconds and milliseconds.
	inline uint64_t total_seconds() const
	{
		return m_mills / 1000;
	}

	inline uint64_t total_millsecs() const
	{
		return m_mills;
	}

private:
	Fmt			m_format;
	uint64_t	m_mills;
	char		m_buffer[32];
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Format
{
	ECO_VALUE_API(Format);
public:
	// constructor, parse date and time from string.
	bool set(IN const char* str);

	// get date.
	const char* get_date() const;

	// get time.
	const char* get_time() const;

public:
	// time trans: "2020-12-03 10:28:28"
	static inline const std::string& std_to_iso(OUT std::string& dtime)
	{
		dtime.erase(16, 1);
		dtime.erase(13, 1);
		dtime.erase(10, 1);
		dtime.erase(7, 1);
		dtime.erase(4, 1);
		return dtime;
	}
	static inline void to_iso_date(OUT std::string& date, Fmt from = fmt_std)
	{
		switch (from)
		{
		case eco::date_time::fmt_iso:
			break;
		case eco::date_time::fmt_std:
			date.erase(7, 1);
			date.erase(4, 1);
			break;
		}
	}

	static inline void iso_date_to(OUT std::string& date, Fmt to = fmt_std)
	{
		switch (to)
		{
		case eco::date_time::fmt_iso:
			break;
		case eco::date_time::fmt_std:
			date.insert(4, 1, '-');
			date.insert(7, 1, '-');
			break;
		}
	}

	static inline void iso_date_to(OUT char* date, Fmt fmt_to = fmt_std)
	{
		switch (fmt_to)
		{
		case eco::date_time::fmt_iso:
			break;
		case eco::date_time::fmt_std:
			eco::insert(date, 4, 1, '-');
			eco::insert(date, 7, 1, '-');
			break;
		}
	}

	static inline void format_date(OUT std::string& date, Fmt from, Fmt to)
	{
		to_iso_date(date, from);
		iso_date_to(date, to);
	}

	static inline void to_std_time(OUT std::string& time, Fmt from = fmt_iso)
	{
		switch (from)
		{
		case eco::date_time::fmt_iso:
		case eco::date_time::fmt_iso_m:
			time.insert(2, 1, ':');
			time.insert(5, 1, ':');
			time.resize(8);
			break;
		case eco::date_time::fmt_std:
			break;
		}
	}

	static inline void std_time_to(OUT std::string& time, Fmt fmt_to = fmt_iso)
	{
		switch (fmt_to)
		{
		case eco::date_time::fmt_iso:
		case eco::date_time::fmt_iso_m:
			time.erase(5, 1);
			time.erase(2, 1);
			time.resize(6);
			break;
		case eco::date_time::fmt_std:
			break;
		}
	}

	static inline void format_time(OUT std::string& time, Fmt from, Fmt to)
	{
		to_std_time(time, from);
		std_time_to(time, to);
	}
};


////////////////////////////////////////////////////////////////////////////////
/* make object id by timestamp.
ver1_id = 3bit-ver + 13bit-front + 32bit-timestamp + 16bit-seq.
		= 8          8192          4294967296(136year) 65536‬
means: this system can make 536870912=8192 * 65536(0.54 billion) object
each second for 136 * 8 years. and it can redefine by 8 times.
start time: 20200101 00:00:00
*/
ECO_API uint64_t make_id_by_ver1(uint16_t front, uint32_t& ts, uint32_t& seq);

////////////////////////////////////////////////////////////////////////////////
}}
#endif