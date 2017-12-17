#include "PrecHeader.h"
#include "FileSink.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/process/Process.h>
#include <eco/net/Net.h>
#include <time.h>



namespace eco{;
namespace log{;



//##############################################################################
//##############################################################################
Context::Context(
	IN const std::string& file_path,
	IN uint64_t roll_size,
	IN uint32_t flush_interval,
	IN bool is_utc)
	: m_file_path(file_path)
	, m_roll_size(roll_size)
	, m_flush_interval(flush_interval)
	, m_is_utc(false)
{
	update();
	m_start_day = get_tm(m_now_time)->tm_yday;
	m_last_time = m_now_time;
}


////////////////////////////////////////////////////////////////////////////////
void Context::update()
{
	m_last_time = m_now_time;
	m_now_time = ::time(nullptr);
}


////////////////////////////////////////////////////////////////////////////////
tm* Context::get_tm(IN const time_t& t) const
{
	return (!m_is_utc) ? ::localtime(&t) : ::gmtime(&t);
}


////////////////////////////////////////////////////////////////////////////////
const std::string Context::get_file_path() const
{
	std::string fullpath;
	fullpath.reserve(m_file_path.size() + 64);
	fullpath = m_file_path;

	// file name format: "20160204-150303.pc-zhouyu.2087.log"
	char buf[32];
	strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S.", get_tm(m_now_time));
	fullpath += buf;
	fullpath += eco::net::get_hostname();
	fullpath += ".";
	fullpath += eco::this_process::get_id_string();
	fullpath += ".log";
	return fullpath;
}


////////////////////////////////////////////////////////////////////////////////
bool Context::is_roll_file(IN const uint64_t cur_size) const
{
	if (cur_size > m_roll_size)
	{
		return true;
	}

	if (m_start_day < get_tm(m_now_time)->tm_yday)
	{
		return true;
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////
bool Context::is_flush() const
{
	return (m_now_time - m_last_time) >= m_flush_interval;
}


////////////////////////////////////////////////////////////////////////////////
void Context::roll_file()
{
	m_start_day = get_tm(m_now_time)->tm_yday;
}



//##############################################################################
//##############################################################################
FileSink::FileSink(
	IN const std::string& file_path,
	IN uint64_t roll_size,
	IN uint32_t flush_interval,
	IN bool is_utc,
	IN OnChangedLogFile& func)
	: m_context(file_path, roll_size, flush_interval, is_utc)
	, m_on_changed(func)
{
	// open mode: append text file.
	m_file.open(m_context.get_file_path().c_str(), "a");
}


////////////////////////////////////////////////////////////////////////////////
void FileSink::append(IN const char* buf, IN uint32_t size)
{
	m_file.write(buf, size);

	m_context.update();
	if (m_context.is_roll_file(m_file.file_size()))
	{
		std::string new_file_path(m_context.get_file_path());
		m_file.open(new_file_path.c_str(), "a");
		m_context.roll_file();
		if (m_on_changed != nullptr)
		{
			m_on_changed(new_file_path.c_str());
		}
	}

	if (m_context.is_flush())
	{
		m_file.flush();
	}
}


}}

