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
#include <eco/ExportApi.h>
#include <eco/Memory.h>
#include <eco/date_time/Date.h>
#include <eco/date_time/Time.h>


namespace eco{;
namespace date_time{;
////////////////////////////////////////////////////////////////////////////////
class DateTime
{
public:
	inline DateTime(Date& date, uint32_t time) : m_date(date), m_time(time)
	{}

    inline DateTime(const std::string& time)
    {
        m_time = Time::get_second(&time[9]);
        m_date = eco::date_time::Date(time);
    }

	inline uint32_t seconds() const
	{
		return m_time;
	}

	inline uint32_t days() const
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
		return m_date.days() * Time::day_seconds() + m_time;
	}

private:
	Time m_time;
	Date m_date;
};
// get date and time of now. time second: "00:00:00" -> 0.
ECO_API DateTime now();


////////////////////////////////////////////////////////////////////////////////
class ECO_API Timestamp
{
public:
	/*
	time_format: time text format.
	second_clock: get time from a second clock precision.
	*/
	inline explicit Timestamp(IN Fmt time_format = fmt_isot)
	{
		m_millsecs = 0;
		m_time_format = time_format;
		m_timestamp[0] = '\0';
		if (time_format == fmt_iso_m || 
			time_format == fmt_isot_m ||
			time_format == fmt_std_m)
		{
			set_clock(false);
		}
		else
		{
			set_clock(true);
		}
	}

	inline const char* get_time() const
	{
		switch (m_time_format)
		{
		case eco::date_time::fmt_iso:
		case eco::date_time::fmt_iso_m:
			return &m_timestamp[8];
		case eco::date_time::fmt_isot:
		case eco::date_time::fmt_isot_m:
			return &m_timestamp[9];
		case eco::date_time::fmt_std:
		case eco::date_time::fmt_std_m:
		default:
			break;
		}
		return &m_timestamp[11];
	}

	inline operator const char*() const
	{
		return m_timestamp;
	}

	inline const char* get_value() const
	{
		return m_timestamp;
	}

	inline std::string get_date() const
	{
		std::string val;
		switch (m_time_format)
		{
		case eco::date_time::fmt_iso:
		case eco::date_time::fmt_iso_m:
		case eco::date_time::fmt_isot:
		case eco::date_time::fmt_isot_m:
			return val.assign(m_timestamp, 8);
		case eco::date_time::fmt_std:
		case eco::date_time::fmt_std_m:
		default:
			break;
		}
		return val.assign(m_timestamp, 10);
	}

	inline bool operator==(IN const Timestamp& ts) const
	{
		return ts.m_time_format == m_time_format
			&& strcmp(ts.m_timestamp, m_timestamp) == 0;
	}
	inline bool operator!=(IN const Timestamp& ts) const
	{
		return !(operator==(ts));
	}
	inline bool operator<(IN const Timestamp& ts) const
	{
		return ts.m_time_format == m_time_format
			&& strcmp(ts.m_timestamp, m_timestamp) < 0;
	}

	inline static Date today()
	{
		eco::date_time::Timestamp ts(fmt_isot);
		return eco::date_time::Date(ts.get_date());
	}

	// total seconds and milliseconds.
	inline uint64_t total_seconds() const
	{
		return m_millsecs / 1000;
	}

	inline uint64_t total_millsecs() const
	{
		return m_millsecs;
	}

private:
	void set_clock(IN bool second_clock);
	
	uint64_t m_millsecs;
	char m_timestamp[32];
	Fmt m_time_format;
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
const uint32_t year_days = 365;
const uint32_t month_days = 30;


/* count duration size.
year: "2018-01-31" - "2017-12-31" = 1/12
year: "2017-07-31" - "2017-01-31" = 0.5
*/
ECO_API double get_duration_size(
	IN const char* start_dt,
	IN const char* end_dt,
	IN TimeUnitType t_type);

/* count duration unit.
year: "2018-01-31" - "2017-12-31" = 1
year: "2017-07-31" - "2017-01-31" = 0
*/
ECO_API int get_duration_unit(
	IN const char* start_dt,
	IN const char* end_dt,
	IN const TimeUnitType t_type);


/* get time slice.
exp1: start_dt="2017-07-14 09:30:00",t_unit=0,t_type=minute :
"2017-07-14 09:30:00"
exp2: start_dt="2017-07-14 09:30:00",t_unit=1,t_type=minute :
"2017-07-14 09:35:00"
*/
ECO_API std::string get_unit_time(
	IN const char* start_dt,
	IN const uint32_t t_unit,
	IN const TimeUnitType t_type);


/* format date time to date time unit.
minute: 2017-02-25 12:25:00
hour:	2017-02-25 12:00:00
day:	2017-02-25 00:00:00
week:	2017-03-05 00:00:00		sunday
month:	2017-03-31 00:00:00		end day of month
year:	2017-12-31 00:00:00		end day of year
*/
ECO_API void format_date_time_unit(
	OUT std::string& dt,
	IN const TimeUnitType t_type);

// is history date time: date2 & time2.
inline bool history(
	IN uint32_t time1, IN const std::string& date1,
	IN uint32_t time2, IN const std::string& date2)
{
	return date1 > date2 || date1 == date2 && time1 > time2;
}

// make session id by version.
ECO_API uint64_t make_id_by_ver1(
	uint16_t front, uint32_t& ts, uint32_t& seq);


////////////////////////////////////////////////////////////////////////////////
}}
#endif