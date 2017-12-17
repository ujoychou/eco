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
#include <eco/Btask.h>


ECO_NS_BEGIN(eco);


////////////////////////////////////////////////////////////////////////////////
class ECO_API TimerObject
{
	ECO_VALUE_API(TimerObject);
public:
	// cancel timer operation.
	void cancel();

	// is null.
	bool empty();

	// release timer.
	void release();
};


typedef std::function<void(IN const bool is_cancel)> OnTimerFunc;
////////////////////////////////////////////////////////////////////////////////
class ECO_API Timer
{
	ECO_OBJECT_API(Timer);
public:
	// start timer and it's worker.
	void start();
	void join();

	// stop timer and it's worker.
	void stop();

	/*@ add timer for dedicated duration.
	*/
	TimerObject add_timer(
		IN const uint32_t millsecs,
		IN const bool repeated,
		IN const Btask& task);
	TimerObject add_timer(
		IN const uint32_t millsecs,
		IN const bool repeated,
		IN std::auto_ptr<Btask>& task);
	TimerObject add_timer(
		IN const uint32_t millsecs,
		IN const bool repeated,
		IN OnTimerFunc task);

	/*@ add timer for dedicated date time.
	* @ para.date_time: format as "2015-03-02 12:12:12".
	*/
	void add_timer(
		IN const char* date_time,
		IN const Btask& task);
	void add_timer(
		IN const char* date_time,
		IN std::auto_ptr<Btask>& task);
	void add_timer(
		IN const char* date_time,
		IN OnTimerFunc task);

	/*@ add daily timer, and it can repeat.
	* @ para.time: format as "12:12:12".
	*/
	void add_daily_timer(
		IN const char* time,
		IN const bool  repeated,
		IN const Btask& task);
	void add_daily_timer(
		IN const char* time,
		IN const bool  repeated,
		IN std::auto_ptr<Btask>& task);
	void add_daily_timer(
		IN const char* time,
		IN const bool  repeated,
		IN OnTimerFunc task);
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif