#ifndef ECO_THREAD_TIMER_H
#define ECO_THREAD_TIMER_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2017-03-10.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2017 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/Thread/Task.h>
#include <eco/Type.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class ECO_API Timer
{
	ECO_VALUE_API(Timer);
public:
	// cancel timer operation.
	void cancel();

	// is null.
	bool empty();

	// release timer.
	void release();
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API TimerServer
{
	ECO_OBJECT_API(TimerServer);
public:
	// start timer and it's worker.
	void start();
	void join();

	// stop timer and it's worker.
	void stop();

	// add timer for dedicated duration.
	Timer add(IN uint32_t millsecs, IN bool repeated, IN Task::ptr& task);
	Timer add(uint32_t millsecs, bool repeated, Closure&& task);

	/*@ add timer for dedicated date time.
	* @ para.date_time: format as "2015-03-02 12:12:12".
	*/
	Timer add(IN const char* date_time, IN Task::ptr& task);
	Timer add(IN const char* date_time, IN Closure&& task);

	/*@ add daily timer, and it can repeat.
	* @ para.time: format as "12:12:12".
	*/
	Timer add_daily(IN const char* time, IN Task::ptr& task);
	Timer add_daily(const char* time, IN Closure&& task);
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif