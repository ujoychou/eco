#include "PrecHeader.h"
#include <eco/proxy/Proxy.h>
////////////////////////////////////////////////////////////////////////////////
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>



namespace eco{;
namespace proxy{;
////////////////////////////////////////////////////////////////////////////////
bool RunOnceExe(
	IN const std::string& szEXE_FILE,
	IN const std::string& szPARAS,
	IN bool bShow)
{
	// is exe file exist.
	using namespace boost::filesystem;
	boost::system::error_code ec;
	if (!boost::filesystem::exists(path(szEXE_FILE), ec))
	{
		return false;
	}

	// when exe process is not running, run it.
	boost::filesystem::path path(szEXE_FILE);
	if (IsExeFileRunning(szEXE_FILE))
	{
		return true;
	}
	return RunExe(szEXE_FILE, szPARAS, bShow);
}


////////////////////////////////////////////////////////////////////////////////
std::string GetSystemDate()
{
	using namespace boost::gregorian;
	date today = day_clock::local_day();
	return to_iso_extended_string(today);
}


////////////////////////////////////////////////////////////////////////////////
std::string GetSystemTime()
{
	using namespace boost::posix_time;
	return boost::posix_time::to_simple_string(
		second_clock::local_time().time_of_day());
}


////////////////////////////////////////////////////////////////////////////////
std::string GetSystemDateTime()
{
	using namespace boost::posix_time;

	ptime pt = second_clock::local_time();
	std::string szDateTime = boost::posix_time::to_iso_extended_string(pt);
	boost::replace_first(szDateTime, "T", " ");
	return szDateTime;
}


////////////////////////////////////////////////////////////////////////////////
}// ns::proxy
}// ns::eco