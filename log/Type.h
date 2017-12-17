#ifndef ECO_LOG_TYPE_H
#define ECO_LOG_TYPE_H
/*******************************************************************************
@ name
log api type.

@ function
1.


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
#include <eco/Type.h>


namespace eco{;
namespace log{;


////////////////////////////////////////////////////////////////////////////////
enum
{
	trace = 0,
	debug,
	info,
	warn,
	error,
	fatal,
	none,
};
typedef uint16_t SeverityLevel;


////////////////////////////////////////////////////////////////////////////////
enum 
{
	// logging persist in file.
	file_sink = 0x0001,
	// logging display on console.
	console_sink = 0x0002,
};
typedef int SinkOption;


enum 
{
	log_text_size = 256,
	log_pack_size = 8192,
	// min size
	log_min_queue_size = 1 * 1024 * 1024,
	log_min_file_roll_size = 10 * 1024 * 1024,
	log_min_sync_interval = 1000,
	// default size
	log_queue_size = 3 * 1024 * 1024,
	log_file_roll_size = 50 * 1024 * 1024,
	log_sync_interval = 3000,
};


////////////////////////////////////////////////////////////////////////////////
typedef eco::FixBuffer<log_text_size>	Text;
typedef eco::FixBuffer<log_pack_size>	Pack;
typedef eco::FixStream<log_text_size>	LogStream;
typedef void (*OnChangedLogFile)(IN const char* file_path);


}}
#endif