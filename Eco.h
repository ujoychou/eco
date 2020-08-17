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
class TimerServer;
class TaskUnit;
////////////////////////////////////////////////////////////////////////////////
class ECO_API Eco
{
	ECO_SINGLETON_API(Eco);
public:
	// singleton object.
	static Eco& get();

	// post task to executing queue.
	void post_task(IN TaskUnit& task);

	// post task to wait queue.
	void post_wait(IN TaskUnit& task);

	// move wait task to executing.
	void move_wait();

	// get timer.
	TimerServer& timer();

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
ECO_NS_END(eco);
#endif