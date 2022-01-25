#include "Pch.h"
#include <eco/thread/Thread.h>
////////////////////////////////////////////////////////////////////////////////
#include <set>
#include <sstream>
#include <eco/log/Log.h>
#include <eco/rx/RxImpl.h>
#include <eco/std/mutex.h>
#include <eco/std/atomic.h>
#include <eco/std/thread.h>


////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(eco);
std_atomic_uint32_t	 s_thread_count;
thread_local size_t	 t_nid = 0;
thread_local char	 t_sid[16] = { 0 };
thread_local char	 t_name[32] = { 0 };
thread_local Thread::Impl*	t_thread = 0;


////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(this_thread);
// get and format std thread id.
inline const char* init_tid(IN const std::thread::id& id)
{
	std::stringstream ss;
	ss << id;
	t_nid = eco::cast<size_t>(ss.str());
	eco_cpyc(t_sid, eco::Integer<size_t>(t_nid, eco::dec, 8).c_str());
}
inline void init_tid()
{
	if (t_nid == 0) { init_tid(std_this_thread::get_id()); }
}
ECO_NS_END(this_thread);


////////////////////////////////////////////////////////////////////////////////
class DeadLockCheckImpl
{
	ECO_SINGLETON(DeadLockCheckImpl);
public:
	// current timestamp record.
	inline int64_t time() const { return m_time; }
	inline void set_time(uint64_t now_sec) { m_time = now_sec; }

	// lock management.
	void add_lock(eco::this_thread::DeadLock* lock)
	{
		std_lock_guard guard(m_mutex);
		m_lock.insert(lock);
	}
	void del_lock(eco::this_thread::DeadLock* lock)
	{
		std_lock_guard guard(m_mutex);
		m_lock.erase(lock);		
	}

	void on_timer(uint64_t now_sec);

private:
	mutable std_mutex 	m_mutex;
	std_atomic_uint64_t	m_time;
	std::set<eco::this_thread::DeadLock*> m_lock;
};
uint32_t DeadLockCheck::get_time()
{
	return DeadLockCheckImpl::get().time();
}
void DeadLockCheck::on_timer(uint64_t now_sec)
{
	DeadLockCheckImpl::get().on_timer(now_sec);
}


////////////////////////////////////////////////////////////////////////////////
class Thread::Impl
{
public:
	struct DeadLockGuard
	{
		inline DeadLockGuard(this_thread::DeadLock& lock) : m_lock(lock)
		{
			DeadLockCheckImpl::get().add_lock(&m_lock);
			++s_thread_count;
		}
		inline ~DeadLockGuard()
		{
			DeadLockCheckImpl::get().del_lock(&m_lock);
			--s_thread_count;
		}
		this_thread::DeadLock& m_lock;
	};

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

	inline void run(const char* name, std::function<void()>&& func)
	{
		m_name = name;
		m_thread = std_thread([=]() {
			t_thread = m_lock.m_thread;
			eco_cpys(t_name, m_name);
			this_thread::init_tid();
			m_sid = t_sid;
			m_nid = t_nid;
			DeadLockGuard guard(m_lock);
			func();
		});
		// wait thread func executed.
		eco::thread::wait([this]() -> bool { return this->m_nid != 0; });
	}

	std_atomic_size_t 	m_nid;
	std::string 		m_sid;
	std::string 		m_name;
	std_thread  		m_thread;
	this_thread::DeadLock m_lock;
};


////////////////////////////////////////////////////////////////////////////////
ECO_OBJECT_IMPL(Thread);
ECO_PROPERTY_STR_IMPL(Thread, name);
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
void Thread::run(const char* name, std::function<void()>&& func)
{
	impl().run(name, std::move(func));
}
void Thread::join()
{
	if (impl().m_thread.joinable())
	{
		impl().m_thread.join();
	}
}
void DeadLockCheckImpl::on_timer(uint64_t now_sec)
{
	set_time(now_sec);
	std_lock_guard guard(m_mutex);
	for (auto it = m_lock.begin(); it != m_lock.end(); ++it)
	{
		this_thread::DeadLock& lock = **it;
		if (lock.happen(m_time))
		{
			ECO_ERROR << "thread_deadlock" << lock.m_thread->m_name
				<= lock.m_thread->m_sid <= lock.m_stamp <= lock.m_timeout;
		}
	}// end for.	
}


ECO_NS_BEGIN(this_thread);
////////////////////////////////////////////////////////////////////////////////
size_t id()
{
	init_tid();
	return t_nid;
}
const char* sid()
{
	init_tid();
	return t_sid;
}
const char* name()
{
	return t_name;
}
DeadLock& dead_lock()
{
	return t_thread->m_lock;
}
void init()
{
	init_tid();
}
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(this_thread);
ECO_NS_END(eco);