#ifndef ECO_THREAD_H
#define ECO_THREAD_H
/*******************************************************************************
@ name
thread and common thread function.

@ function


@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/Project.h>


namespace eco{;
namespace this_thread{;


////////////////////////////////////////////////////////////////////////////////
// sleep
ECO_API void sleep(IN int millisecond);

// get current thread id.
ECO_API uint64_t id();
ECO_API const char* id_string();

// get current thread name.
ECO_API const char* name();

// init thread.
ECO_API void init();
}}


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
class ECO_API Thread
{
	ECO_MOVABLE_API(Thread);
public:
	// count thread size.
	static uint32_t get_thread_count();

	// get thread id;
	size_t get_id() const;
	const char* get_id_string();

	// thread name.
	void set_name(IN const char*);
	const char* get_name() const;
	Thread& name(IN const char*);

	// start thread.
	typedef std::function<void (void)> thread_func;
	void run(
		IN thread_func func,
		IN const char* name = nullptr);

	// waiting thread over.
	void join();
};



namespace thread{;

// check data state is valid.
typedef std::function<bool (void)> checker_func;
////////////////////////////////////////////////////////////////////////////////
// check and wait data state is valid infinitely.
inline void wait(IN checker_func checker, IN uint32_t unit_mill_sec = 50)
{
	if (unit_mill_sec < 10)
	{
		unit_mill_sec = 10;
	}

	// wait until the data state is valid.
	while (!checker())
	{
		eco::this_thread::sleep(unit_mill_sec);
	}
}


////////////////////////////////////////////////////////////////////////////////
// check and wait data state is valid until time is out.
inline bool time_wait(
	IN checker_func checker,
	IN const uint32_t timeout_mill_secs,
	IN uint32_t unit_mill_sec = 50)
{
	if (unit_mill_sec < 10)
	{
		unit_mill_sec = 10;
	}

	// wait until the data state is valid or when timeout.
	uint32_t count_mill_secs = 0;
	while (!checker() && count_mill_secs < timeout_mill_secs)
	{
		eco::this_thread::sleep(unit_mill_sec);
		count_mill_secs += unit_mill_sec;
	}

	// timeout return false, else true.
	return count_mill_secs < timeout_mill_secs;
}


////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline void release(IN std::shared_ptr<T>& sp, IN int unit_sleep = 50)
{
	std::weak_ptr<T> wp(sp);
	sp.reset();

	while (wp.use_count() > 0)
	{
		eco::this_thread::sleep(unit_sleep);
	}
}
}
////////////////////////////////////////////////////////////////////////////////
}
#endif