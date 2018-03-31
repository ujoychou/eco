#include "PrecHeader.h"
#include <eco/DateTime.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/Type.h>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>



namespace eco{;
namespace date_time{;


//##############################################################################
//##############################################################################
Timestamp::Timestamp(IN const Format fmt)
{
	m_timestamp[0] = '\0';
	m_fmt = fmt;

	using namespace boost::posix_time;
	ptime pt = microsec_clock::local_time();

	// format: "20160510 23:05:05.100200"
	auto dt = pt.date();
	auto ti = pt.time_of_day();
	uint32_t y = dt.year();
	uint32_t m = dt.month();
	uint32_t d = dt.day();
	uint32_t h = static_cast<uint32_t>(ti.hours());
	uint32_t mi = static_cast<uint32_t>(ti.minutes());
	uint32_t s  = static_cast<uint32_t>(ti.seconds());
	uint32_t ms = static_cast<uint32_t>(ti.total_microseconds() % 1000000);

	switch (m_fmt)
	{
	case eco::date_time::fmt_iso:
		snprintf(m_timestamp, sizeof(m_timestamp),
			"%4d%02d%02d%02d%02d%02d",
			y, m, d, h, mi, s);
		break;
	case eco::date_time::fmt_iso_m:
		snprintf(m_timestamp, sizeof(m_timestamp),
			"%4d%02d%02d%02d%02d%02d%06d",
			y, m, d, h, mi, s, ms);
		break;
	case eco::date_time::fmt_isot:
		snprintf(m_timestamp, sizeof(m_timestamp),
			"%4d%02d%02d %02d:%02d:%02d",
			y, m, d, h, mi, s);
		break;
	case eco::date_time::fmt_isot_m:
		snprintf(m_timestamp, sizeof(m_timestamp),
			"%4d%02d%02d %02d:%02d:%02d.%06d",
			y, m, d, h, mi, s, ms);
		break;
	case eco::date_time::fmt_std:
		snprintf(m_timestamp, sizeof(m_timestamp),
			"%4d-%02d-%02d %02d:%02d:%02d",
			y, m, d, h, mi, s);
		break;
	case eco::date_time::fmt_std_m:
	default:
		snprintf(m_timestamp, sizeof(m_timestamp),
			"%4d-%02d-%02d %02d:%02d:%02d.%06d",
			y, m, d, h, mi, s, ms);
		break;
	}
}


////////////////////////////////////////////////////////////////////////////////
const char* Timestamp::get_time() const
{
	switch (m_fmt)
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



//##############################################################################
//##############################################################################
class DateTime::Impl
{
	ECO_IMPL_INIT(DateTime);
public:
	std::string m_date;
	std::string m_time;

public:
	void set_date_time(IN const char* str)
	{
		// 去除空格
		std::string dt_v(str);
		boost::trim(dt_v);

		// 格式为"2014-01-01x00:00:00"
		if (dt_v.size() == 19 &&
			std::count(dt_v.begin(), dt_v.end(), ':') == 2 &&
			std::count(dt_v.begin(), dt_v.end(), '-') == 2)
		{
			m_date = dt_v.substr(0, 10);
			m_time = dt_v.substr(11, 8);
			return;
		}
		// 格式"2014-01-01"：转换为"2014-01-01x00:00:00"
		if (dt_v.size() == 10 &&
			std::count(dt_v.begin(), dt_v.end(), '-') == 2)
		{
			m_date = dt_v;
			m_time = "00:00:00";
			return;
		}
		// 格式"20140101"：转换为"2014-01-01x00:00:00"
		if (dt_v.size() == 8 &&
			std::count(dt_v.begin(), dt_v.end(), ':') == 0)
		{
			dt_v.insert(6, 1, '-');
			dt_v.insert(4, 1, '-');
			m_date = dt_v;
			m_time = "00:00:00";
			return;
		}
		// 格式"00:00:00"：转换为"1900-01-01x00:00:00"
		if (dt_v.size() == 8 &&
			std::count(dt_v.begin(), dt_v.end(), ':') == 2)
		{
			m_date = ("1900-01-01");
			m_time = dt_v;
			return;
		}
		// 格式"000000"：转换为"1900-01-01x00:00:00"
		if (dt_v.size() == 6)
		{
			dt_v.insert(4, 1, ':');
			dt_v.insert(2, 1, ':');
			m_date = ("1900-01-01");
			m_time = dt_v;
			return;
		}
		// 格式"20140101T00:00:00"：转换为"2014-01-01x00:00:00"
		if (dt_v.size() == 17 &&
			std::count(dt_v.begin(), dt_v.end(), ':') == 2 &&
			std::count(dt_v.begin(), dt_v.end(), '-') == 0)
		{
			dt_v.insert(6, 1, '-');
			dt_v.insert(4, 1, '-');
			m_date = dt_v.substr(0, 10);
			m_time = dt_v.substr(11, 8);
			return;
		}
		// 格式"20140101000000"：转换为"2014-01-01x00:00:00"
		if (dt_v.size() == 14 &&
			std::count(dt_v.begin(), dt_v.end(), ':') == 0 &&
			std::count(dt_v.begin(), dt_v.end(), '-') == 0)
		{
			dt_v.insert(8, 1, 'x');
			dt_v.insert(13, 1, ':');
			dt_v.insert(11, 1, ':');
			dt_v.insert(6, 1, '-');
			dt_v.insert(4, 1, '-');
			m_date = dt_v.substr(0, 10);
			m_time = dt_v.substr(11, 8);
			return;
		}
		// 格式"20140101T000000"：转换为"2014-01-01x00:00:00"
		if (dt_v.size() == 15 &&
			std::count(dt_v.begin(), dt_v.end(), ':') == 0 &&
			std::count(dt_v.begin(), dt_v.end(), '-') == 0)
		{
			dt_v.insert(13, 1, ':');
			dt_v.insert(11, 1, ':');
			dt_v.insert(6, 1, '-');
			dt_v.insert(4, 1, '-');
			m_date = dt_v.substr(0, 10);
			m_time = dt_v.substr(11, 8);
			return;
		}

		m_date = "1900-01-01";
		m_time = "00:00:00";
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_VALUE_IMPL(DateTime);
DateTime& DateTime::set(IN const char* str)
{
	m_impl->set_date_time(str);
	return *this;
}
const char* DateTime::get_date() const
{
	return m_impl->m_date.c_str();
}
const char* DateTime::get_time() const
{
	return m_impl->m_time.c_str();
}


////////////////////////////////////////////////////////////////////////////////
double get_duration_size(
	IN const char* s_dt,
	IN const char* e_dt,
	IN TimeUnitType t_type)
{
	double v = 0;
	try
	{
		if (t_type > ttype_day && t_type <= ttype_year)
		{
			auto s = boost::gregorian::from_simple_string(s_dt);
			auto e = boost::gregorian::from_simple_string(e_dt);
			int day_size = (e - s).days();
			if (day_size == 0)
			{
				day_size = 1;
			}
			if (t_type == ttype_year)
			{
				v = day_size / 365.0;
			}
			else if (t_type == ttype_month)
			{
				v = day_size / 30.0;
			}
			else if (t_type == ttype_week)
			{
				v = day_size / 7.0;
			}
		}
		else if (t_type >= ttype_second && t_type <= ttype_day)
		{
			auto s = boost::posix_time::time_from_string(s_dt);
			auto e = boost::posix_time::time_from_string(e_dt);
			if (t_type == ttype_day)
			{
				v = (e - s).hours() / 24.0;
			}
			else if (t_type == ttype_hour)
			{
				v = (double)(e - s).hours();
			}
			else if (t_type == ttype_minute)
			{
				v = (double)(e - s).total_seconds() / 60;
			}
			else if (t_type == ttype_second)
			{
				v = (double)(e - s).total_seconds();
			}
		}
		return v;
	}
	catch (std::exception& e) 
	{
		e.what();
		EcoThrow << "get_duration_size error time: "
			<< s_dt << " " << e_dt;
	}
	EcoThrow << "get_duration_size error type:" << t_type;
	return v;
}


////////////////////////////////////////////////////////////////////////////////
int get_duration_unit(
	IN const char* s_dt,
	IN const char* e_dt,
	IN const TimeUnitType t_type)
{
	int v = 0;
	try 
	{
		auto s = boost::gregorian::from_simple_string(s_dt);
		auto e = boost::gregorian::from_simple_string(e_dt);
		if (t_type <= ttype_year && t_type >= ttype_day)
		{
			if (t_type == ttype_year)
			{
				v = e.year() - s.year();
			}
			else if (t_type == ttype_season)
			{
				int sm = s.year() * 12 + s.month();
				int em = e.year() * 12 + e.month();
				int m = (em - sm) / 3;
				int seq = s.month() % 3;
				v = m + (3 + (em - sm) % 3 + seq) / 3 - 1;
			}
			else if (t_type == ttype_month)
			{
				int em = e.year() * 12 + e.month();
				int sm = s.year() * 12 + s.month();
				v = em - sm;
			}
			else if (t_type == ttype_week)
			{
				int  d = (e - s).days();
				int  seq = s.day_of_week();
				seq = (seq != 0) ? seq - 1 : 7 - 1;
				v = d / 7 + (7 + d % 7 + seq) / 7 - 1;
			}
			else if (t_type == ttype_day)
			{
				v = (e - s).days();
			}
		}
		else if (t_type <= ttype_hour && t_type >= ttype_second)
		{
			int d = (e - s).days();
			if (t_type <= ttype_hour)
			{
				int h = atoi(&e_dt[11]) - atoi(&s_dt[11]);
				v = h + d * 24;
			}
			if (t_type <= ttype_minute)
			{
				int m = atoi(&e_dt[14]) - atoi(&s_dt[14]);
				v = v * 60;
				v = v + m;
			}
			if (t_type <= ttype_second)
			{
				int s = atoi(&e_dt[17]) - atoi(&s_dt[17]);
				v = v * 60;
				v = v + s;
			}
		}
		return v;
	}
	catch (std::exception& e)
	{
		e.what();
		EcoThrow << "get_duration_unit error time: "
			<< s_dt << " " << e_dt;
	}
	EcoThrow << "get_duration_unit error type:" << t_type;
	return v;
}


////////////////////////////////////////////////////////////////////////////////
std::string get_unit_time(
	IN const char* start_dt,
	IN const uint32_t t_unit,
	IN const TimeUnitType t_type)
{
	try
	{
		if (t_unit == 0)
		{
			std::string dt = start_dt;
			format_date_time_unit(dt, t_type);
			return dt;
		}

		if (t_type <= ttype_year && t_type >= ttype_day)
		{
			auto s = boost::gregorian::from_simple_string(start_dt);
			switch (t_type)
			{
			case ttype_year:
				s += boost::gregorian::years(t_unit);
				break;
			case ttype_month:
				s += boost::gregorian::months(t_unit);
				break;
			case ttype_day:
				s += boost::gregorian::days(t_unit);
				break;
			}
			std::string fmt = boost::gregorian::to_iso_extended_string(s);
			fmt += " 00:00:00";
			return fmt;
		}
		else if (t_type <= ttype_hour && t_type >= ttype_second)
		{
			auto s = boost::posix_time::time_from_string(start_dt);
			switch (t_type)
			{
			case ttype_hour:
				s += boost::posix_time::hours(t_unit);
				break;
			case ttype_minute:
				s += boost::posix_time::minutes(t_unit);
				break;
			case ttype_second:
				s += boost::posix_time::seconds(t_unit);
				break;
			}
			std::string fmt = boost::posix_time::to_iso_extended_string(s);
			fmt[10] = ' ';
			format_date_time_unit(fmt, t_type);
			return fmt;
		}
	}
	catch (std::exception& e)
	{
		e.what();
		EcoThrow << "get_unit_time error time: " << start_dt;
	}
	assert(false);
	return "";
}


////////////////////////////////////////////////////////////////////////////////
void reset_number(OUT char* dt, IN  const char ch = '0')
{
	for (; *dt != '\0'; ++dt)
	{
		if (*dt <= '9' && *dt > '0')
			*dt = ch;
	}
}
ECO_API void format_date_time_unit(
	OUT std::string& dt, IN const TimeUnitType type)
{
	switch (type)
	{
	case ttype_second:		// 2017-06-13 09:30:30
		break;
	case ttype_minute:		// 2017-06-13 09:30:00
		reset_number(&dt[16]);
		break;
	case ttype_hour:		// 2017-06-13 09:00:00
		reset_number(&dt[13]);
		break;
	case ttype_day:			// 2017-06-13 00:00:00
		reset_number(&dt[10]);
		break;
	case ttype_week:		// 2017-06-13 00:00:00
	case ttype_month:		// 2017-06-13 00:00:00
	case ttype_season:		// 2017-06-13 00:00:00
	case ttype_year:		// 2017-06-13 00:00:00
		reset_number(&dt[10]);
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
}}