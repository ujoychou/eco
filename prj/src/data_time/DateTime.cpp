#include "Pch.h"
#include <eco/date_time/DateTime.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/date_time/Time.h>


ECO_NS_BEGIN(eco);
namespace date_time{;
////////////////////////////////////////////////////////////////////////////////
class Format::Impl
{
	ECO_IMPL_INIT(Format);
public:
	std::string m_date;
	std::string m_time;

public:
	bool set_date_time(IN const char* str)
	{
		if (str == nullptr) str = eco::value_empty.c_str();

		// 去除空格
		std::string dt_v(str);
		//boost::trim(dt_v);

		// 格式为"2014-01-01x00:00:00"
		if (dt_v.size() == 19 &&
			std::count(dt_v.begin(), dt_v.end(), ':') == 2 &&
			std::count(dt_v.begin(), dt_v.end(), '-') == 2)
		{
			m_date = dt_v.substr(0, 10);
			m_time = dt_v.substr(11, 8);
			return true;
		}
		// 格式"2014-01-01"：转换为"2014-01-01x00:00:00"
		if (dt_v.size() == 10 &&
			std::count(dt_v.begin(), dt_v.end(), '-') == 2)
		{
			m_date = dt_v;
			m_time = "00:00:00";
			return true;
		}
		// 格式"20140101"：转换为"2014-01-01x00:00:00"
		if (dt_v.size() == 8 &&
			std::count(dt_v.begin(), dt_v.end(), ':') == 0)
		{
			dt_v.insert(6, 1, '-');
			dt_v.insert(4, 1, '-');
			m_date = dt_v;
			m_time = "00:00:00";
			return true;
		}
		// 格式"00:00:00"：转换为"1900-01-01x00:00:00"
		if (dt_v.size() == 8 &&
			std::count(dt_v.begin(), dt_v.end(), ':') == 2)
		{
			m_date = ("1900-01-01");
			m_time = dt_v;
			return true;
		}
		// 格式"000000"：转换为"1900-01-01x00:00:00"
		if (dt_v.size() == 6)
		{
			dt_v.insert(4, 1, ':');
			dt_v.insert(2, 1, ':');
			m_date = ("1900-01-01");
			m_time = dt_v;
			return true;
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
			return true;
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
			return true;
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
			return true;
		}

		m_date = "1900-01-01";
		m_time = "00:00:00";
		return false;
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_VALUE_IMPL(Format);
bool Format::set(IN const char* str)
{
	return m_impl->set_date_time(str);
}
const char* Format::get_date() const
{
	return m_impl->m_date.c_str();
}
const char* Format::get_time() const
{
	return m_impl->m_time.c_str();
}


////////////////////////////////////////////////////////////////////////////////
uint64_t make_id_by_ver1(uint16_t front, uint32_t& ts, uint32_t& seq)
{
	const uint8_t ver = 0;
	const uint32_t max_seq = (1 << 16);
	eco::date_time::DateTime start(
		eco::date_time::Date(2020, 1, 1),
		eco::date_time::Time(false));

	// get timestamp and seq.
	while (ts == 0 || seq + 1 == max_seq)
	{
		eco::date_time::DateTime curr = eco::date_time::now();
		auto ts_curr = uint32_t(curr.total_seconds() - start.total_seconds());
		if (ts == 0)
		{
			ts = ts_curr;
			seq = 0;
			break;
		}
		else if (ts_curr > ts)
		{
			++ts;
			seq = 0;
			break;
		}
		eco::this_thread::sleep(1000);
	}

	uint64_t result = ver;
	result = result << 13;
	result += front;
	result = result << 32;
	result += ts;
	result = result << 16;
	return result += seq++;
}


////////////////////////////////////////////////////////////////////////////////
}}