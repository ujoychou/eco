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
#include <eco/Project.h>



namespace eco{;
namespace net{;

class IoService;
////////////////////////////////////////////////////////////////////////////////
class Worker
{
	ECO_MOVABLE_API(Worker);
public:
	void run();

	void join();

	void stop();

	void async_stop();

	IoService* get_io_service();
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif