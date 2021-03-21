#include "PrecHeader.h"
#include <eco/thread/Thread.h>
////////////////////////////////////////////////////////////////////////////////
#include <set>
#include <sstream>
#include <eco/cpp/Thread.h>


////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(eco);
std_atomic_uint32_t	 s_thread_count;
thread_local size_t	 t_nid = 0;
thread_local char	 t_sid[16] = { 0 };
thread_local char	 t_name[32] = { 0 };
thread_local Thread::Impl*	t_thread = 0;
// thread local error logging and format.
thread_local eco::Error* t_error = 0;
thread_local eco::FormatX* t_format = 0;
thread_local eco::log::PusherT<eco::String>* t_logbuf = 0;

////////////////////////////////////////////////////////////////////////////////
inline eco::Error& local_error() {
	if (t_error == 0) t_error = new eco::Error();
	return *t_error;
}
inline eco::FormatX& local_format() {
	if (t_format == 0) t_format = new eco::FormatX();
	return *t_format;
}
inline eco::log::PusherT<eco::String>& local_logbuf() {
	if (t_logbuf == 0) t_logbuf = new eco::log::PusherT<eco::String>;
	return *t_logbuf;
}


////////////////////////////////////////////////////////////////////////////////
class ThreadLockGuard
{
public:
	inline ThreadLockGuard(ThreadLock& lock) : m_lock(lock)
	{
		ThreadCheck::get().add_lock(&m_lock);
		++s_thread_count;
	}
	inline ~ThreadLockGuard()
	{
		ThreadCheck::get().del_lock(&m_lock);
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
	std_thread  m_thread;
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
	inline static const char* this_thread_init(IN const std_thread::id& id)
	{
		std::stringstream ss;
		ss << id;
		t_nid = eco::cast<size_t>(ss.str());
		eco_cpyc(t_sid, eco::Integer<size_t>(t_nid, eco::dec, 8).c_str());
		return t_sid;
	}
	inline static const char* this_thread_init()
	{
		return !eco::empty(t_sid) ? t_sid
			: this_thread_init(std_this_thread::get_id());
	}

	inline void run(
		IN std::function<void()>& func,
		IN const char* name_)
	{
		m_name = name_;
		m_thread = std_thread([=] {
			t_thread = m_lock.m_thread;
			eco_cpys(t_name, m_name);
			m_sid = this_thread_init();
			ThreadLockGuard lg(m_lock);
			func();
		});
		m_sid = this_thread_init(m_thread.get_id());
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
void Thread::run(std::function<void()>&& func, IN const char* name_)
{
	impl().run(std::move(func), name_);
}
void Thread::join()
{
	if (impl().m_thread.joinable())
	{
		impl().m_thread.join();
	}
}


ECO_NS_BEGIN(this_thread);
////////////////////////////////////////////////////////////////////////////////
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
eco::Error& error()
{
	return local_error();
}
eco::FormatX& format()
{
	return local_format();
}
eco::FormatX& format(const char* msg)
{
	local_format().reset(msg);
	return local_format();
}
eco::log::PusherT<eco::String>& logbuf()
{
	local_logbuf().stream().clear();
	return local_logbuf();
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
	std_mutex m_mutex;
	std_atomic_int64_t  m_time;
	std::set<ThreadLock*> m_lock;

	inline void init(ThreadCheck&)
	{
		m_time = 0;
	}

	inline void set_time(uint64_t now_sec)
	{
		m_time = now_sec;
	}

	inline void on_timer(uint64_t now_sec)
	{
		set_time(now_sec);
		std_lock_guard lg(m_mutex);
		for (auto it = m_lock.begin(); it != m_lock.end(); ++it)
		{
			// timeout thread has locked.
			ThreadLock& lock = **it;
			if (!lock.unlocked() && lock.timeout(m_time))
			{
				ECO_ERROR << "thread lock " << lock.m_thread->m_sid
					<= lock.m_thread->m_name <= lock.m_object
					<= lock.m_stamp.load() <= lock.m_timeout;
			}
		}// end for.
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_SINGLETON_IMPL(ThreadCheck);
ThreadCheck& ThreadCheck::get()
{
	return eco::Singleton<ThreadCheck>::get();
}
void ThreadCheck::add_lock(ThreadLock* lock)
{
	std_lock_guard lg(impl().m_mutex);
	impl().m_lock.insert(lock);
}
void ThreadCheck::del_lock(ThreadLock* lock)
{
	std_lock_guard lg(impl().m_mutex);
	impl().m_lock.erase(lock);
}
int64_t ThreadCheck::get_time() const
{
	return impl().m_time;
}
void ThreadCheck::set_time(uint64_t now_sec)
{
	impl().set_time(now_sec);
}
void ThreadCheck::on_timer(uint64_t now_sec)
{
	impl().on_timer(now_sec);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);