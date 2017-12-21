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
	void set_severity_level(IN const char* v);
	void set_severity_level(IN const SeverityLevel& v);
	SeverityLevel& severity_level();
	const SeverityLevel& get_severity_level() const;
	Core& severity_level(IN const SeverityLevel&);

	/*@ option: synchronous.*/
	void set_sync(IN const bool);
	bool sync() const;
	Core& sync(IN const bool);

	/*@ set message queue capacity.*/
	void set_capacity(IN const uint32_t& v);
	uint32_t& capacity();
	const uint32_t& get_capacity() const;
	Core& capacity(IN const uint32_t&);

	/*@ set message queue max sync interval mill seconds.*/
	void set_max_sync_interval(IN const uint32_t& v);
	uint32_t& max_sync_interval();
	const uint32_t& get_max_sync_interval() const;
	Core& max_sync_interval(IN const uint32_t&);

	/*@ option: sink option.*/
	void set_sink_option(IN const SinkOption&);
	SinkOption& sink_option();
	const SinkOption& get_sink_option() const;
	Core& sink_option(IN const SinkOption&);

	/*@ option: add\remove sink option.*/
	void add_file_sink(IN bool is_add);
	bool has_file_sink() const;
	void add_console_sink(IN bool is_add);
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
	void set_file_roll_size(IN const uint32_t&);
	uint32_t& file_roll_size();
	const uint32_t& get_file_roll_size() const;
	Core& file_roll_size(IN const uint32_t&);

	/*@ option: file sink flush interval seconds.*/
	void set_flush_interval(IN const uint32_t&);
	uint32_t& flush_interval();
	const uint32_t& get_flush_interval() const;
	Core& flush_interval(IN const uint32_t&);

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
	void append(IN const Text& buf);
};


ECO_API Core& get_core();
////////////////////////////////////////////////////////////////////////////////
}}
#endif