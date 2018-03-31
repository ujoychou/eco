#ifndef ECO_LOG_CORE_H
#define ECO_LOG_CORE_H
/*******************************************************************************
@ name
log core.

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
#include <eco/Object.h>



namespace eco{;
namespace log{;



////////////////////////////////////////////////////////////////////////////////
class ECO_API Core
{
	ECO_IMPL_API();
	ECO_SINGLETON_UNINIT(Core);
public:
	/*@ option: severity level. */
	void set_severity_level(IN const char* v, IN const int flag = 0);
	void set_severity_level(IN const SeverityLevel v, IN const int flag = 0);
	const SeverityLevel get_severity_level() const;

	/*@ option: synchronous.*/
	void set_async(IN const bool);
	bool async() const;
	Core& async(IN const bool);

	/*@ set message queue capacity.*/
	void set_capacity(IN const uint32_t v);
	uint32_t capacity();
	const uint32_t get_capacity() const;
	Core& capacity(IN const uint32_t);

	/*@ set message queue max sync interval mill seconds.*/
	void set_async_flush(IN const uint32_t v);
	uint32_t async_flush();
	const uint32_t get_async_flush() const;
	Core& async_flush(IN const uint32_t);

	/*@ option: sink option.*/
	void set_sink_option(IN const SinkOption);
	SinkOption sink_option();
	const SinkOption get_sink_option() const;
	Core& sink_option(IN const SinkOption);

	/*@ option: add\remove sink option.*/
	void add_file_sink(IN bool);
	bool has_file_sink() const;
	void add_console_sink(IN bool);
	bool has_console_sink() const;

	/*@ option: file sink file name.*/
	void set_file_path(IN const char*);
	const char* get_file_path() const;
	Core& file_path(IN const char*);

	/*@ option: file sink file name.*/
	void set_file_name(IN const char*);
	const char* get_file_name() const;
	Core& file_name(IN const char*);

	/*@ option: file sink file size, bytes.*/
	void set_file_roll_size(IN const uint32_t);
	uint32_t file_roll_size();
	const uint32_t get_file_roll_size() const;
	Core& file_roll_size(IN const uint32_t);

	/*@ file sink: logging file changed callback.*/
	void set_file_on_create(
		IN eco::log::OnChangedLogFile& func);

	/*@ run logging.*/
	void run();

	/*@ stop logging.*/
	void stop();

	// join this logging thread.
	void join();

	/*@ is logging running.*/
	bool is_running() const;

	/*@ append log info.*/
	void append(IN const eco::Bytes& buf, IN const SeverityLevel level);
};


ECO_API Core& get_core();
////////////////////////////////////////////////////////////////////////////////
}}
#endif