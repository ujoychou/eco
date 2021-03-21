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
#include <eco/Object.h>
#include <eco/eco/Proto.h>
#include <list>
#include <eco/cpp/Thread.h>
#include <eco/cpp/Chrono.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(log);
template<typename stream_t> class PusherT;
ECO_NS_END(log);
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
	void run(
		IN std::function<void()>&& func,
		IN const char* name = "");

	// compatible with thread pool.
	inline void run(
		IN std::function<void()>&& func,
		IN uint32_t thread_size,
		IN const char* name = "")
	{
		run(std::move(func), name);
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
		IN std::function<void()>&& func,
		IN uint32_t thread_size,
		IN const char* name = "")
	{
		std::string namei;
		for (uint32_t i = 0; i < thread_size; ++i)
		{
			eco::Thread thr;
			namei = name;
			namei += eco::cast<std::string>(i);
			thr.run(std::move(func), namei.c_str());
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
// timer will check the lock state in intervals.
class ThreadLock;
class ECO_API ThreadCheck
{
	ECO_SINGLETON_API(ThreadCheck);
public:
	void set_time(uint64_t now_sec);
	int64_t get_time() const;
	static ThreadCheck& get();

private:
	void on_timer(uint64_t now_sec);
	void add_lock(ThreadLock*);
	void del_lock(ThreadLock*);
	friend class ThreadLockGuard;
	friend class ThreadCheckOut;
};


////////////////////////////////////////////////////////////////////////////////
// when thread locked, tracer will produce warn.
class ThreadLock
{
public:
	inline ThreadLock(int timeout = 10)
		: m_object(0), m_stamp(0)
		, m_timeout(timeout), m_thread(0)
	{}
	inline ~ThreadLock()
	{}

	// the function of thread begin.
	inline void set_object(uint64_t object_id)
	{
		m_object = object_id;
		m_stamp = ThreadCheck::get().get_time();
	}

	// the function of thread end.
	inline void unlock()
	{
		m_stamp = 0;
		m_object = 0;
	}

	inline bool unlocked() const
	{
		return m_stamp == 0;
	}

private:
	// the function of thread end.
	inline void lock(int64_t stamp)
	{
		m_stamp = stamp;
	}

	// update time at regular intervals.
	inline bool timeout(int64_t curr) const
	{
		return curr - m_stamp >= (int64_t)m_timeout;
	}

	int					m_timeout;			// timeout seconds.
	uint64_t			m_object;			// object id.
	std_atomic_int64_t	m_stamp;			// seconds of day.
	Thread::Impl*		m_thread;			// thread.
	friend class ThreadCheck;
	friend class Thread::Impl;
};


ECO_NS_BEGIN(this_thread);

////////////////////////////////////////////////////////////////////////////////
// get current thread id.
ECO_API size_t id();
ECO_API const char* get_id();

// get current thread name.
ECO_API const char* name();

// get current thread lock.
ECO_API ThreadLock& lock();

/* get current thread format: 
1.return char*: locale get_error.
2.format buffer: 
*/
ECO_API eco::FormatX& format();
ECO_API eco::FormatX& format(const char* msg);
#ifndef ECO_NO_PROTOBUF
ECO_NS_BEGIN(proto);
ECO_API eco::proto::Error& error();
ECO_NS_END(proto);
#endif

// get current thread logging buffer.
ECO_API eco::log::PusherT<eco::String>& logbuf();

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



ECO_NS_BEGIN(thread);
////////////////////////////////////////////////////////////////////////////////
// check and wait data state is valid infinitely.
inline void wait(
	IN std::function<bool()>&& checker,
	IN uint32_t unit_mill_sec = 50)
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
	IN std::function<bool()>&& checker,
	IN uint32_t timeout_mill_secs,
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
inline void release(IN std::weak_ptr<T>& wp, IN int unit_sleep = 10)
{
	while (wp.use_count() > 0)
	{
		if (unit_sleep > 0)
			std_this_thread::sleep_for(std_chrono::milliseconds(unit_sleep));
		else
			std_this_thread::yield();
	}
}
template<typename T>
inline void release(IN std::shared_ptr<T>& sp, IN int unit_sleep = 10)
{
	std::weak_ptr<T> wp(sp);
	sp.reset();
	release(wp, unit_sleep);
}

////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(thread);
ECO_NS_END(eco);
#endif