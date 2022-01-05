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
#include <eco/thread/TimingWheel.h>


ECO_NS_BEGIN(eco);
class Task;
class Btask;
ECO_NS_BEGIN(net);
class MessageHandler;
ECO_NS_END(net);
////////////////////////////////////////////////////////////////////////////////
class ECO_API Eco
{
	ECO_SINGLETON_API(Eco);
public:
	// singleton object.
	static Eco& get();

	// post task to executing queue.
	void post_task(IN Closure&& task);
	void post_task(IN std::shared_ptr<Task>& task);
	void post_task(IN std::shared_ptr<Btask>& task);
	// post btask for vc100
	void post_btask(IN std::shared_ptr<Btask>& task);

	// post task to wait queue.
	void post_wait(IN std::shared_ptr<Btask>&& task);

	// move wait task to executing.
	void move_wait();

	// get timer wheel.
	eco::TimingWheel& timer();

public:
	// async management post item.
	typedef std::shared_ptr<net::MessageHandler> HandlerPtr;
	uint32_t post_async(IN HandlerPtr& hdl);

	// has async message handler.
	bool has_async(IN uint32_t req_id);

	// erase async handler.
	void erase_async(IN uint32_t req_id);

	// pop async message handler.
	HandlerPtr pop_async(IN uint32_t req_id, IN bool last);
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif