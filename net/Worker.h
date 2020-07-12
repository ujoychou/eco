#ifndef ECO_NET_WORKER_H
#define ECO_NET_WORKER_H
/*******************************************************************************
@ name


@ function


@ exception


@ remark


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-12.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Export.h>



namespace eco{;
namespace net{;
class IoWorker;
class IoService;
////////////////////////////////////////////////////////////////////////////////
class Worker
{
	ECO_OBJECT_API(Worker);
public:
	void run(const char* name);

	void join();

	void stop();

	bool stopped() const;
	bool running() const;
	bool initing() const;

	void async_stop();

	IoWorker* get_io_worker();
	IoService* get_io_service();
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif