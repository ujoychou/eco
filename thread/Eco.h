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
#include <eco/rx/RxApi.h>
#include <eco/rx/RxExport.h>
#include <eco/thread/Timing.h>


ECO_NS_BEGIN(eco);
class Btask;
////////////////////////////////////////////////////////////////////////////////
class ECO_API Eco
{
	ECO_SINGLETON_API(Eco);
public:
	// singleton object.
	static Eco& get();

	// post task to executing queue.
	void post_task(IN eco::Task&& task, IN uint32_t restart_sec_if_fail);

	// post btask for vc100
	void post_task(IN std::shared_ptr<Btask>&& task);

	// post task to wait queue.
	void post_wait(IN std::shared_ptr<Btask>&& task);

	// move wait task to executing.
	void move_wait();

	// get timer wheel.
	eco::Timing& timing();	
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif