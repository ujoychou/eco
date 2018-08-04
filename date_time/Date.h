#ifndef ECO_DATE_H
#define ECO_DATE_H
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
#include <eco/Cast.h>
#include <eco/Object.h>
#include <string>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(date_time);
////////////////////////////////////////////////////////////////////////////////
enum
{
	week_sunday			= 0,
	week_monday			= 1,
	week_tuesday		= 2,
	week_wednesday		= 3,
	week_thursday		= 4,
	week_friday			= 5,
	week_saturday		= 6,
	week_day			= 7,

	invalid_julian_day	= -1,
};
typedef uint32_t WeekDay;


////////////////////////////////////////////////////////////////////////////////
class Date : public eco::Value<Date>
{
public:
	struct Ymd
	{
		int year;
		int month;
		int day;
	};

	/* get julian day. 
	note: algorithm and explanation see:
	http://www.faqs.org/faqs/calendars/faq/part2/
	http://blog.csdn.net/Solstice
	char require_32_bit_integer[sizeof(int) >= sizeof(int32_t) ? 1 : -1];
	(void)require_32_bit_integer_at_least; // no warning please
	*/
	inline static int get_day(int year, int month, int day)
	{
		int a = (14 - month) / 12;
		int y = year + 4800 - a;
		int m = month + 12 * a - 3;
		int r = day + (153 * m + 2) / 5 + y * 365;
		return  r + y / 4 - y / 100 + y / 400 - 32045;
	}

	inline static Date::Ymd get_ymd(int day)
	{
		int a = day + 32044;
		int b = (4 * a + 3) / 146097;
		int c = a - ((b * 146097) / 4);
		int d = (4 * c + 3) / 1461;
		int e = c - ((1461 * d) / 4);
		int m = (5 * e + 2) / 153;
		Date::Ymd ymd;
		ymd.day = e - ((153 * m + 2) / 5) + 1;
		ymd.month = m + 3 - 12 * (m / 10);
		ymd.year = b * 100 + d - 4800 + (m / 10);
		return ymd;
	}

	inline static int week(int day)
	{
		return day / eco::date_time::week_day;
	}

	inline static int week_day(int day)
	{
		return (day + 1) % eco::date_time::week_day;
	}

	inline static bool week_end(int day)
	{
		auto week_day = Date::week_day(day);
		return (week_day == week_sunday || week_day == week_saturday);
	}

	inline static int get_day_start()
	{
		return get_day(1970, 1, 1);
	}

////////////////////////////////////////////////////////////////////////////////
public:
	inline Date() : m_days(invalid_julian_day)
	{}

	inline Date(int year, int month, int day)
	{
		m_days = get_day(year, month, day);
	}

	inline explicit Date(const Ymd& v)
	{
		m_days = get_day(v.year, v.month, v.day);
	}

	inline explicit Date(int day) : m_days(day)
	{}

	inline explicit Date(const struct tm& t)
	{
		m_days = get_day(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
	}

	inline explicit Date(const char* t)
	{
		int y = eco::cast<int16_t>(&t[0], 4);
		int m = eco::cast<int16_t>(&t[4], 2);
		int d = eco::cast<int16_t>(&t[6], 2);
		m_days = get_day(y, m, d);
	}

	inline explicit Date(const std::string& t)
	{
		int y = eco::cast<int16_t>(&t[0], 4);
		int m = eco::cast<int16_t>(&t[4], 2);
		int d = eco::cast<int16_t>(&t[6], 2);
		m_days = get_day(y, m, d);
	}

	// ÔËËã·û+/-
	inline Date& operator+=(int day)
	{
		m_days += day;
		return *this;
	}
	inline Date& operator-=(int day)
	{
		m_days -= day;
		return *this;
	}

	inline void swap(Date& that)
	{
		std::swap(m_days, that.m_days);
	}

	// format julian day to ios string like "20170506";
	inline std::string to_ios_string() const
	{
		char buf[32];
		Ymd ymd = get_ymd(m_days);
		snprintf(buf, sizeof(buf), "%4d%02d%02d", ymd.year, ymd.month, ymd.day);
		return buf;
	}

	// format julian day to std string like "2017-05-06";
	inline std::string to_std_string() const
	{
		char buf[32];
		Ymd ymd = get_ymd(m_days);
		snprintf(buf, sizeof(buf), "%4d-%02d-%02d", ymd.year, ymd.month, ymd.day);
		return buf;
	}

	inline Ymd ymd() const
	{
		return get_ymd(m_days);
	}

	// get day of week. 
	// [0, 1, ..., 6] => [Sunday, Monday, ..., Saturday ]
	inline int week_day() const
	{
		return week_day(m_days);
	}
	inline int week_day7() const
	{
		int wd = week_day(m_days);
		if (wd == 0) wd = 7;
		return wd;
	}

	inline int week() const
	{
		return week(m_days);
	}

	// get julian day.
	inline int days() const
	{ 
		return m_days; 
	}

	// get monday.
	inline uint32_t get_monday()
	{
		return m_days - (week_day7() - 1);
	}

private:
	int m_days;
};


////////////////////////////////////////////////////////////////////////////////
inline bool operator<(Date x, Date y)
{
	return x.days() < y.days();
}
inline bool operator==(Date x, Date y)
{
	return x.days() == y.days();
}


////////////////////////////////////////////////////////////////////////////////
inline int get_season(int month)
{
	return (month - 1) / 3 + 1;
}
inline int get_season_start_month(int month)
{
	return (get_season(month) - 1) * 3 + 1;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(date_time);
ECO_NS_END(eco);
#endif
