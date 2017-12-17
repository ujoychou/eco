#ifndef ECO_THREAD_POOL_H
#define ECO_THREAD_POOL_H
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
#include <eco/thread/Thread.h>
#include <list>



namespace eco{;


////////////////////////////////////////////////////////////////////////////////
class ThreadPool
{
public:
	// start thread.
	typedef eco::Thread::thread_func thread_func;
	void run(
		thread_func func,
		IN uint32_t thread_size,
		IN const char* name = nullptr)
	{
		for (uint32_t i=0; i<thread_size; ++i)
		{
			std::auto_ptr<Thread> thr(new Thread);
			thr->run(func, name);
			m_threads.push_back(thr.release());
		}
	}

	// waiting thread over.
	void join()
	{
		for (auto it = m_threads.begin(); it != m_threads.end(); ++it)
		{
			(**it).join();
		}
		release();
	}

	// release
	void release()
	{
		std::list<eco::Thread*>::iterator it;
		for (it = m_threads.begin(); it != m_threads.end(); ++it)
		{
			delete *it;
		}
		m_threads.clear();
	}

	~ThreadPool()
	{
		release();
	}

private:
	std::list<eco::Thread*> m_threads;
};


////////////////////////////////////////////////////////////////////////////////
}
#endif