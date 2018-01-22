#include "PrecHeader.h"
#ifdef ECO_LINUX
#include <eco/thread/Thread.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/ExportImpl.h>
// linux header.
#include <pthread.h>
#include <unistd.h>



// this thread data.
////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace this_thread{;
__thread size_t	t_tid;
__thread char[64]	t_tid_string;
__thread char[16]	t_tname;

void sleep(IN int millisecond)
{
	usleep(millisecond * 1000);		// micro seconds.
}
size_t get_id()
{
	return t_tid;
}
const char* get_id_string()
{
	return t_tid_string;
}
}}



namespace eco{;
namespace thread{;
////////////////////////////////////////////////////////////////////////////////
class Thread::Impl
{
public:
	pthread_t m_tid;
	std::string m_tid_string;
	std::string m_name;
	Thread::thread_func m_func;
	static eco::Atomic<uint32_t> s_thread_count;
	static void work(IN void* tdata);
};


////////////////////////////////////////////////////////////////////////////////
ECO_SHARED_IMPL(Thread);
ECO_PROPERTY_STR_IMPL(Thread, name);
eco::Atomic<uint32_t> Thread::Impl::s_thread_count;
static void Thread::Impl::work(IN void* tdata)
{
	try 
	{
		Thread::Impl* impl = (Thread()::Impl*)tdata;

		// thead id.
		eco::Integer caster(impl().m_tid, eco::dec, 8);
		impl().m_tid_string = caster.c_str();
		t_tid = impl().m_tid;
		strcpy(t_tid_string, impl().m_tid_string.c_str());

		// run thread func.
		impl().m_func();
	}
	catch (const std::exception& e)
	{
		EcoFatal << "exception caught in thread " << t_thread_id 
			<< "," << m_name << ": " << e.what();
	}
}
Thread()
{
	++Thread::Impl::s_thread_count;
}
uint32_t Thread::get_thread_count()
{
	return Thread::Impl::s_thread_count;
}
int64_t Thread::get_id() const
{
	return m_impl->get_data().m_tid;
}
const char* Thread::get_id_string()
{
	return m_impl->get_data().m_tid_string;
}
void Thread::start(thread_func func)
{
	if (0 != pthread_create(&m_impl->data().m_tid, 0, &Impl::work, m_impl))
	{
		throw std::logic_error("create pthread error!");
	}
}
void Thread::join()
{
	if (0 != pthread_join(&m_impl->data().m_tid, 0))
	{
		throw std::logic_error("join pthread error!");
	}
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif