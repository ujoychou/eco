#include "PrecHeader.h"
#include <eco/thread/Thread.h>
////////////////////////////////////////////////////////////////////////////////
#include <set>
#include <mutex>
#include <thread>
#include <sstream>
#include <chrono>
#include <eco/Cast.h>
#include <eco/log/log.h>


ECO_NS_BEGIN(eco);
std::atomic_int		 s_thread_count = 0;
thread_local size_t	 t_nid = 0;
thread_local char	 t_sid[16] = { 0 };
thread_local char	 t_name[32] = { 0 };
thread_local Thread::Impl* t_thread = 0;


////////////////////////////////////////////////////////////////////////////////
class ThreadLockGuard
{
public:
	inline ThreadLockGuard(ThreadLock& lock) : m_lock(lock)
	{
		ThreadCheck::me().add_lock(&m_lock);
		++s_thread_count;
	}
	inline ~ThreadLockGuard()
	{
		ThreadCheck::me().del_lock(&m_lock);
		--s_thread_count;
	}
	ThreadLock& m_lock;
};


////////////////////////////////////////////////////////////////////////////////
class Thread::Impl
{
public:
	size_t m_nid;
	std::string m_sid;
	std::string m_name;
	std::thread m_thread;
	eco::ThreadLock	m_lock;

public:
	inline void init(Thread& t)
	{
		m_lock.m_thread = this;
		m_nid = 0;
	}

	inline ~Impl()
	{
		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}

	// get and format std thread id.
	static void this_thread_init(IN const std::thread::id& id)
	{
		std::stringstream ss;
		ss << id;
		t_nid = eco::cast<size_t>(ss.str());
		eco_cpyc(t_sid, eco::Integer<size_t>(t_nid, eco::dec, 8).c_str());
	}
	inline static void this_thread_init()
	{
		if (eco::empty(t_sid))
			this_thread_init(std::this_thread::get_id());
	}

	inline void run(
		IN std::function<void()>& func,
		IN const char* name)
	{
		m_name = name;
		m_thread = std::thread([=] {
			t_thread = m_lock.m_thread;
			eco_cpys(t_name, m_name);
			this_thread_init();
			ThreadLockGuard lg(m_lock);
			func();
		});
		this_thread_init(m_thread.get_id());
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_OBJECT_IMPL(Thread);
ECO_PROPERTY_STR_IMPL(Thread, name);
////////////////////////////////////////////////////////////////////////////////
uint32_t Thread::count()
{
	return s_thread_count;
}
size_t Thread::id() const
{
	return impl().m_nid;
}
const char* Thread::get_id() const
{
	return impl().m_sid.c_str();
}
void Thread::run(std::function<void()>&& func, IN const char* name)
{
	impl().run(std::move(func), name);
}
void Thread::join()
{
	if (impl().m_thread.joinable())
	{
		impl().m_thread.join();
	}
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(this_thread);
size_t id()
{
	Thread::Impl::this_thread_init();
	return t_nid;
}
const char* get_id()
{
	Thread::Impl::this_thread_init();
	return t_sid;
}
const char* name()
{
	return t_name;
}
ThreadLock& lock()
{
	return t_thread->m_lock;
}
void init()
{
	Thread::Impl::this_thread_init();
}
ECO_NS_END(this_thread);

////////////////////////////////////////////////////////////////////////////////
class ThreadCheck::Impl
{
public:
	std::mutex m_mutex;
	std::atomic_int64_t  m_time;
	std::set<ThreadLock*> m_lock;

	inline void init(ThreadCheck&)
	{
		set_time();
	}

	inline void set_time()
	{
		using namespace std::chrono;
		auto d = steady_clock::now().time_since_epoch();
		seconds t = duration_cast<seconds>(d);
		m_time = t.count();
	}

	inline void on_timer()
	{
		set_time();
		std::lock_guard<std::mutex> lg(m_mutex);
		for (auto it = m_lock.begin(); it != m_lock.end(); ++it)
		{
			// timeout thread has locked.
			ThreadLock& lock = **it;
			if (!lock.unlocked() && lock.timeout(m_time))
			{
				ECO_ERROR << "thread lock " << lock.m_thread->m_sid
					<= lock.m_thread->m_name <= lock.m_object
					<= lock.m_stamp <= lock.m_timeout;
			}
		}// end for.
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_SINGLETON_IMPL(ThreadCheck);
ThreadCheck& ThreadCheck::me()
{
	return eco::Singleton<ThreadCheck>::instance();
}
void ThreadCheck::add_lock(ThreadLock* lock)
{
	std::lock_guard<std::mutex> lg(impl().m_mutex);
	impl().m_lock.insert(lock);
}
void ThreadCheck::del_lock(ThreadLock* lock)
{
	std::lock_guard<std::mutex> lg(impl().m_mutex);
	impl().m_lock.erase(lock);
}
int64_t ThreadCheck::get_time() const
{
	return impl().m_time;
}
void ThreadCheck::set_time()
{
	impl().set_time();
}
void ThreadCheck::on_timer()
{
	impl().on_timer();
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);