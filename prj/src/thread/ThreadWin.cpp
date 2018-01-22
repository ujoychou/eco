#include "PrecHeader.h"
#ifdef ECO_WIN32
#include <eco/thread/Thread.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/Cast.h>
#include <eco/thread/Atomic.h>
#include <eco/log/Log.h>
#include <windows.h>
#include <process.h>



// this thread data.
namespace eco{;
namespace this_thread{;

__declspec(thread) uint64_t	t_tid = 0;
__declspec(thread) char		t_tid_string[16] = {0};
__declspec(thread) char		t_tname[16] = "--";
////////////////////////////////////////////////////////////////////////////////
void sleep(IN int millisecond)
{
	::Sleep(millisecond);
}
uint64_t id()
{
	return t_tid;
}
const char* id_string()
{
	return t_tid_string;
}
const char* name()
{
	return t_tname;
}
void init()
{
	t_tid = GetCurrentThreadId();
	eco_cpyc(t_tid_string, eco::Integer<uint64_t>(t_tid, eco::dec, 8));
	eco::empty(t_tname);
}
}}



namespace eco{;
////////////////////////////////////////////////////////////////////////////////
class Thread::Impl
{
public:
	uint64_t m_handle;
	std::string m_tid_string;
	std::string m_name;
	Thread::thread_func m_func;
	static eco::Atomic<uint32_t> s_thread_count;

public:
	void init(Thread&);
	void run(thread_func& func, IN const char* name);
	~Impl();
	void join();
};


////////////////////////////////////////////////////////////////////////////////
ECO_MOVABLE_IMPL(Thread);
ECO_PROPERTY_STR_IMPL(Thread, name);
eco::Atomic<uint32_t> Thread::Impl::s_thread_count;
void Thread::Impl::join()
{
	if (m_handle != -1)
	{
		DWORD dw = WaitForSingleObject((HANDLE)m_handle, INFINITE);
		if (WAIT_OBJECT_0 != dw)
		{
			EcoError << "join thread error: " << this_thread::id_string()
				<< '.' << m_name << ": " << ::GetLastError();
		}
		m_handle = -1;
	}
}
////////////////////////////////////////////////////////////////////////////////
void Thread::Impl::init(Thread&)
{
	m_handle = -1;
	m_func = nullptr;
	++s_thread_count;
}
Thread::Impl::~Impl()
{
	join();
	--s_thread_count;
}


////////////////////////////////////////////////////////////////////////////////
unsigned __stdcall work(IN void* tdata)
{
	Thread::Impl& impl = *(Thread::Impl*)tdata;
	try 
	{
		// init this_thread info.
		eco::this_thread::t_tid = impl.m_handle;
		strcpy_s(eco::this_thread::t_tid_string, 16, impl.m_tid_string.c_str());
		strcpy_s(eco::this_thread::t_tname, 16, impl.m_name.c_str());

		// run thread func.
		impl.m_func();
	}
	catch (eco::Error& e)
	{
		EcoError << e;
	}
	catch (const std::exception& e)
	{
		EcoError << "thread func error: " << this_thread::id_string()
			<< '.' << impl.m_name << ": " << e.what();
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
void Thread::Impl::run(thread_func& func, IN const char* name)
{
	m_func = func;
	if (name != nullptr)
	{
		m_name = name;
	}

	// create thread.
	m_handle = ::_beginthreadex(NULL, 0, &work, this, CREATE_SUSPENDED, NULL);
	if (m_handle == 0)
	{
		EcoThrow(::GetLastError()) << "create win thread error!";
	}
	m_tid_string = eco::Integer<uint64_t>(m_handle, eco::dec, 8);

	// resume thread.
	DWORD result = ::ResumeThread((HANDLE)m_handle);
	if (result == -1)
	{
		EcoThrow(::GetLastError()) << "resume win thread error!";
	}
}


////////////////////////////////////////////////////////////////////////////////
uint32_t Thread::get_thread_count()
{
	return Thread::Impl::s_thread_count;
}
size_t Thread::get_id() const
{
	return static_cast<size_t>(impl().m_handle);
}
const char* Thread::get_id_string()
{
	return impl().m_tid_string.c_str();
}
void Thread::run(thread_func func, IN const char* name)
{
	impl().run(func, name);
}
void Thread::join()
{
	m_impl->join();
}


////////////////////////////////////////////////////////////////////////////////
}
#endif