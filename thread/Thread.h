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
#include <eco/Cast.h>
#include <eco/rx/RxApi.h>
#include <eco/rx/RxExport.h>
#include <eco/std/thread.h>
#include <eco/std/chrono.h>
#include <list>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class ECO_API Thread
{
	ECO_OBJECT_API(Thread);
public:
	// count thread size.
	static uint32_t count();

	// get thread id;
	size_t id() const;
	const char* get_id() const;

	// thread name.
	void set_name(IN const char*);
	const char* name() const;
	Thread& name(IN const char*);

	// start thread.
	void run(IN const char* name, IN eco::Task&& func);

	// compatible with thread pool.
	inline void run(
		IN const char* name,
		IN uint32_t thread_size,
		IN eco::Task&& func)
	{
		run(name, std::move(func));
	}

	// waiting thread over.
	void join();
};


////////////////////////////////////////////////////////////////////////////////
class ThreadPool
{
public:
	// start thread.
	inline void run(
		IN const char* name,
		IN uint32_t thread_size,
		IN eco::Task&& func)
	{
		std::string namei;
		for (uint32_t i = 0; i < thread_size; ++i)
		{
			eco::Thread thr;
			namei = name;
			namei += eco::cast(i);
			thr.run(namei.c_str(), std::move(func));
			m_threads.push_back(std::move(thr));
		}
	}

	// get thread size.
	inline size_t size() const
	{
		return m_threads.size();
	}

	// waiting thread over.
	inline void join()
	{
		for (auto it = m_threads.begin(); it != m_threads.end(); ++it)
		{
			it->join();
		}
	}

private:
	std::list<eco::Thread> m_threads;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(this_thread);
// get current thread id.
ECO_API size_t id();
ECO_API const char* get_id();

// get current thread name.
ECO_API const char* name();

// get current thread lock.
class DeadLock;
ECO_API DeadLock& dead_lock();

// init thread.
ECO_API void init();

inline void yield()
{
	std_this_thread::yield();
}
inline void sleep(IN int millisecond)
{
	std_this_thread::sleep_for(std_chrono::milliseconds(millisecond));
}
ECO_NS_END(this_thread);


////////////////////////////////////////////////////////////////////////////////
class DeadLockCheckImpl;
class ECO_API DeadLockCheck
{
public:
	// time unit: second. no need to use high precision unit.
	// get current time in dead lock check. return day of time seconds.
	static uint32_t get_time();

	// update dead lock check time. set day of time seconds.
	static void on_timer(uint64_t now);
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(this_thread);
// when thread locked, tracer will produce warn.
class DeadLock
{
public:
	inline DeadLock(int timeout = 10)
		: m_stamp(0), m_timeout(timeout), m_thread(nullptr) {}

	// the function of thread begin.
	// update time at regular intervals.
	inline void begin() { m_stamp = eco::DeadLockCheck::get_time(); }
	// the function of thread end.
	inline void end() { m_stamp = 0; }

	// check whether deadlock happen.
	inline bool happen(int64_t curr) const
	{
		bool is_begin = (m_stamp != 0);
		bool is_timeout = (curr - m_stamp >= (int64_t)m_timeout);
		return is_begin && is_timeout;
	}

private:
	friend class eco::DeadLockCheckImpl;
	friend class eco::Thread::Impl;
	uint32_t m_stamp;			// seconds of day.
	uint32_t m_timeout;			// timeout seconds.
	Thread::Impl* m_thread;		// thread.
};
ECO_NS_END(this_thread);


ECO_NS_BEGIN(thread);
////////////////////////////////////////////////////////////////////////////////
// check and wait data state is valid infinitely.
inline void wait(IN std::function<bool()>&& checker, IN uint32_t sleep_ms = 50)
{
	// wait until the data state is valid.
	while (!checker())
	{
		if (sleep_ms == 0)
			std_this_thread::yield();
		else
			std_this_thread::sleep_for(std_chrono::milliseconds(sleep_ms));
	}
}
// check and wait data state is valid until time is out.
inline bool time_wait(
	IN std::function<bool()>&& checker,
	IN uint32_t timeout_ms, IN uint32_t sleep_ms = 50)
{
	// wait until the data state is valid or when timeout.
	uint32_t count_ms = 0;
	if (sleep_ms < 10) { sleep_ms = 10; }
	while (!checker() && count_ms < timeout_ms)
	{
		eco::this_thread::sleep(sleep_ms);
		count_ms += sleep_ms;
	}
	// timeout return false, else true.
	return count_ms < timeout_ms;
}


////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline void release(IN std::weak_ptr<T>& wp, IN int sleep_ms = 10)
{
	while (wp.use_count() > 0)
	{
		if (sleep_ms > 0)
			std_this_thread::sleep_for(std_chrono::milliseconds(sleep_ms));
		else
			std_this_thread::yield();
	}
}
template<typename T>
inline void release(IN std::shared_ptr<T>& sp, IN int sleep_ms = 10)
{
	std::weak_ptr<T> wp(sp);
	sp.reset();
	release(wp, sleep_ms);
}
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(thread);
ECO_NS_END(eco);
#endif