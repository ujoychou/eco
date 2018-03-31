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
#include <string>


namespace eco{;
namespace date_time{;


////////////////////////////////////////////////////////////////////////////////
enum Format
{
	// iso: "20160510230505".
	fmt_iso			= 0x0001,
	// iso_m: "20160510230505100200".
	fmt_iso_m		= 0x0002,
	// iso: "20160510 23:05:05".
	fmt_isot		= 0x0003,
	// iso: "20160510 23:05:05.688675".
	fmt_isot_m		= 0x0004,
	// std: "2016-05-10 23:05:05".
	fmt_std			= 0x0005,
	// std: "2016-05-10 23:05:05.688675".
	fmt_std_m		= 0x0006,
};


// time unit type.
enum TimeUnitType
{
	ttype_second		= 1,
	ttype_minute		= 2,
	ttype_hour			= 3,
	ttype_day			= 4,
	ttype_week			= 5,
	ttype_month			= 6,
	ttype_season		= 7,
	ttype_year			= 8,
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Timestamp
{
public:
	explicit Timestamp(
		IN const Format fmt = fmt_iso_m);

	const char* get_time() const;

	inline operator const char*() const
	{
		return m_timestamp;
	}

	inline const char* Timestamp::get_value() const
	{
		return m_timestamp;
	}

	inline std::string get_date() const
	{
		std::string val;
		switch (m_fmt)
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
		return ts.m_fmt == m_fmt && strcmp(ts.m_timestamp, m_timestamp) == 0;
	}
	inline bool operator!=(IN const Timestamp& ts) const
	{
		return !(operator==(ts));
	}
	inline bool operator<(IN const Timestamp& ts) const
	{
		return ts.m_fmt == m_fmt && strcmp(ts.m_timestamp, m_timestamp) < 0;
	}

private:
	char m_timestamp[32];
	Format m_fmt;
};


////////////////////////////////////////////////////////////////////////////////
inline std::string today(IN const Format fmt = fmt_std)
{
	return Timestamp(fmt).get_date();
}


////////////////////////////////////////////////////////////////////////////////
inline void to_iso_date(
	IN OUT std::string& date,
	IN const Format fmt_from = fmt_std)
{
	switch (fmt_from)
	{
	case eco::date_time::fmt_iso:
		break;
	case eco::date_time::fmt_std:
		date.erase(7, 1);
		date.erase(4, 1);
		break;
	}
}

inline void iso_date_to(
	IN OUT std::string& date,
	IN const Format fmt_to = fmt_std)
{
	switch (fmt_to)
	{
	case eco::date_time::fmt_iso:
		break;
	case eco::date_time::fmt_std:
		date.insert(4, 1, '-');
		date.insert(7, 1, '-');
		break;
	}
}

inline void iso_date_to(
	IN OUT char* date,
	IN const Format fmt_to = fmt_std)
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

inline std::string format_date(
	IN OUT std::string& date,
	IN const Format fmt_from,
	IN const Format fmt_to)
{
	to_iso_date(date, fmt_from);
	iso_date_to(date, fmt_to);
}


////////////////////////////////////////////////////////////////////////////////
inline void to_std_time(
	IN OUT std::string& time,
	IN const Format fmt_from = fmt_iso)
{
	switch (fmt_from)
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
inline void std_time_to(
	IN OUT std::string& time,
	IN const Format fmt_to = fmt_iso)
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
inline std::string format_time(
	IN OUT std::string& time,
	IN const Format fmt_from,
	IN const Format fmt_to)
{
	to_std_time(time, fmt_from);
	std_time_to(time, fmt_to);
}


////////////////////////////////////////////////////////////////////////////////
class ECO_API DateTime
{
	ECO_VALUE_API(DateTime);
public:
	// constructor, parse date and time from string.
	DateTime& set(IN const char* str);

	// get date.
	const char* get_date() const;

	// get time.
	const char* get_time() const;
};


////////////////////////////////////////////////////////////////////////////////
const uint32_t year_days = 365;
const uint32_t month_days = 30;


////////////////////////////////////////////////////////////////////////////////
inline uint32_t get_season(IN const uint32_t month_num)
{
	return (month_num - 1) / 3 + 1;
}

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


////////////////////////////////////////////////////////////////////////////////
}}
#endif