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
#include <eco/rx/RxApi.h>
#include <eco/rx/RxExport.h>
#include <eco/log/Type.h>
#include <eco/String.h>
#include <eco/Object.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(log);
////////////////////////////////////////////////////////////////////////////////
class ECO_API Core
{
	ECO_SINGLETON_API(Core);
public:
	// singleton object.
	static Core& get();

	/*@ option: severity level. */
	void set_severity_level(IN const char* v, IN int flag = 0);
	void set_severity_level(IN SeverityLevel v, IN int flag = 0);
	SeverityLevel severity_level() const;

	/*@ option: synchronous.*/
	void set_async(IN bool);
	bool async() const;
	Core& async(IN bool);

	/*@ set message queue capacity.*/
	void set_capacity(IN uint32_t v);
	uint32_t& get_capacity();
	uint32_t capacity() const;
	Core& capacity(IN uint32_t);

	/*@ set message queue max sync interval mill seconds.*/
	void set_async_flush(IN uint32_t v);
	uint32_t& get_async_flush();
	uint32_t async_flush() const;
	Core& async_flush(IN uint32_t);

	/*@ option: sink option.*/
	void set_sink_option(IN SinkOption);
	SinkOption& get_sink_option();
	SinkOption sink_option() const;
	Core& sink_option(IN SinkOption);

	/*@ option: add\remove sink option.*/
	void add_file_sink(IN bool);
	bool has_file_sink() const;
	void add_console_sink(IN bool);
	bool has_console_sink() const;

	/*@ option: file sink file path.*/
	void set_file_path(IN const char*);
	const char* file_path() const;
	Core& file_path(IN const char*);

	/*@ option: file sink file size, bytes.*/
	void set_file_roll_size(IN uint32_t);
	uint32_t& get_file_roll_size();
	uint32_t file_roll_size() const;
	Core& file_roll_size(IN uint32_t);

	/*@ file sink: logging file changed callback.*/
	void set_file_on_create(IN eco::log::OnChangedLogFile& func);

	/*@ run logging.*/
	void run();

	/*@ stop logging.*/
	void stop();

	// join this logging thread.
	void join();

	/*@ append log info.*/
	void append(IN const eco::Bytes& buf, IN SeverityLevel level);
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(log);
ECO_NS_END(eco);
#endif