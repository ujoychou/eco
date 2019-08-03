#ifndef ECO_ECO_H
#define ECO_ECO_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2017-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2017 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Type.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
class MessageHandler;
ECO_NS_END(net);
class Task;
class Timer;
////////////////////////////////////////////////////////////////////////////////
class ECO_API Eco
{
	ECO_OBJECT_API(Eco);
public:
	// post task to executing queue.
	void post_task(IN std::shared_ptr<Task>& task);

	// post task to wait queue.
	void post_wait(IN std::shared_ptr<Task>& task);

	// move wait task to executing.
	void move_wait();

	// get timer.
	Timer& timer();

public:
	// async management post item.
	uint32_t post_async(
		IN std::shared_ptr<net::MessageHandler>& hdl);

	// has async message handler.
	bool has_async(IN uint32_t req_id);

	// erase async handler.
	void erase_async(IN uint32_t req_id);

	// pop async message handler.
	std::shared_ptr<net::MessageHandler> pop_async(
		IN uint32_t req_id, IN bool last);
};


////////////////////////////////////////////////////////////////////////////////
ECO_API Eco& eco();
ECO_API bool has_eco();
ECO_NS_END(eco);
#endif