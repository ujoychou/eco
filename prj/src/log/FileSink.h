#ifndef ECO_FILE_SINK_H
#define ECO_FILE_SINK_H
/*******************************************************************************
@ name
sink.

@ function

@ exception

@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/log/Type.h>
#include <eco/filesystem/File.h>
#include <time.h>



namespace eco{;
namespace log{;


////////////////////////////////////////////////////////////////////////////////
class Context : public eco::Object<Context>
{
public:
	Context(
		IN const std::string& file_path,
		IN uint64_t roll_size,
		IN uint32_t flush_interval,
		IN bool is_utc);

	// update time.
	void update();

	// transform time format: utc or local time.
	tm*  get_tm(IN const time_t& t) const;

	// is flush to file.
	bool is_flush() const;

	const std::string get_file_path() const;

	// roll file: 1)every day; 2)file size > roll size.
	bool is_roll_file(
		IN const uint64_t cur_size) const;

	void roll_file();

private:
	std::string m_file_path;

	uint64_t m_roll_size;
	uint32_t m_flush_interval;		// flush seconds.
	bool m_is_utc;
	
	int m_start_day;
	time_t m_last_time;
	time_t m_now_time;
};


////////////////////////////////////////////////////////////////////////////////
class FileSink : public eco::Object<FileSink>
{
public:
	FileSink(
		IN const std::string& file_path,
		IN uint64_t roll_size,
		IN uint32_t flush_interval,
		IN bool is_utc,
		IN eco::log::OnChangedLogFile& func);

	// append buffer to file.
	void append(IN const char* buf, IN uint32_t size);


private:
	eco::filesystem::File m_file;
	Context m_context;
	eco::log::OnChangedLogFile m_on_changed;
};


}}
#endif