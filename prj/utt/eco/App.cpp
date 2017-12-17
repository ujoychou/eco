#include "PrecHeader.h"
#include "App.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/log/Log.h>
#include <eco/Being.h>
#include <eco/thread/Thread.h>




namespace eco{;
namespace cmd{;
namespace test{;
////////////////////////////////////////////////////////////////////////////////
void task_func()
{
	EcoInfo << "task func.";
}
void App::task_func(IN const uint32_t v, IN bool is_cancel)
{
	if (is_cancel)
	{
		static uint32_t i = 0;
		EcoInfo << "task func, cancelled: " << v << ' ' << ++i;
	}
	else
	{
		EcoInfo << "task func: " << v;
	}
}

uint32_t g_int = 0;
////////////////////////////////////////////////////////////////////////////////
class TaskA : public eco::Task
{
	ECO_TASK(TaskA, 1);
public:
	inline TaskA()
	{}

	virtual const eco::TaskState occupy() override
	{
		if (eco::has(g_int, 1))
		{
			return task_finished;
		}
		return task_occupied;
	}

	virtual void execute() override
	{
		eco::add(g_int, 1);
		EcoDebug << "task a execute.";
	}

private:
	uint32_t m_state;
	uint32_t m_prepare;
};


////////////////////////////////////////////////////////////////////////////////
class TaskB : public eco::Task
{
	ECO_TASK(TaskB, 2);
public:
	inline TaskB()
	{}

	virtual const eco::TaskState occupy() override
	{
		if (eco::has(g_int, 2))
		{
			return task_finished;
		}
		else if (eco::has(g_int, 1))
		{
			return task_occupied;
		}
		return task_no_ready;
	}

	// execute pre_task operation.
	virtual void prepare() override
	{
		EcoDebug << "task b prepare.";
		TaskA task;
		task.start();
		EcoDebug << "task b prepare end.";
	}

	// do execute task operation.
	virtual void execute() override
	{
		eco::add(g_int, 2);
		EcoDebug << "task b execute.";
	}

private:
	uint32_t m_state;
	uint32_t m_prepare;
};


////////////////////////////////////////////////////////////////////////////////
class NormalBeing : public eco::Being
{
public:
	NormalBeing() : eco::Being(1)
	{}

	~NormalBeing()
	{
		EcoInfo << "being on exit.";
	}

	virtual bool on_born() override
	{
		EcoInfo << "being on born.";
		return true;
	}
	virtual void on_live() override
	{
		EcoInfo << "being on live.";
	}
};


////////////////////////////////////////////////////////////////////////////////
class ReleaseBeing : public eco::Being
{
public:
	ReleaseBeing() : eco::Being(1)
	{}

	virtual bool on_born() override
	{
		EcoInfo << "being2 on born.";
		return true;
	}
	virtual void on_live() override
	{
		EcoInfo << "being2 on live.";
		NormalBeing be;
		be.born();

		//delete this;
		//EcoInfo << "release self on live deleted.";
	}
};


////////////////////////////////////////////////////////////////////////////////
class MeBeing : public eco::Being
{
public:
	MeBeing() : eco::Being(1)
	{}

	virtual void on_live() override
	{
		EcoInfo << "being3 on live.";
		EcoInfo << "being3 delete begin.";
		delete this;
		EcoInfo << "being3 delete end.";
	}
};
NormalBeing g_being;
ReleaseBeing g_release_being;
MeBeing g_me_being;


////////////////////////////////////////////////////////////////////////////////
void BeingTest()
{
	for (auto i = 0; i < 2000; ++i)
	{
		NormalBeing be1;
		ReleaseBeing be2;
		be1.born();
		be2.born();
		//eco::this_thread::sleep(30);
	}

	EcoError << "being test over.";
}


////////////////////////////////////////////////////////////////////////////////
App::App()
{}


////////////////////////////////////////////////////////////////////////////////
void App::on_init()
{
	// #1.任务链
	if (false)
	{
		TaskB taskb;
		taskb.start();
	}

	// #2.定时器
	if (false)
	{
		m_timer.start();
		if (false)
		{
			eco::TimerObject obj = m_timer.add_timer(3000, false,
				std::bind(&App::task_func, this, 'I', std::placeholders::_1));
			eco::this_thread::sleep(2000);
			obj.cancel();
			return;
		}
		// 2.1.间隔定时
		m_timer.add_timer(1000, false, std::bind(
			&App::task_func, this, 'I', std::placeholders::_1));
		m_timer.add_timer(2000, true, std::bind(
			&App::task_func, this, 'I', std::placeholders::_1));
		m_timer.add_timer(1000, false, TaskA());
		m_timer.add_timer(2000, true, TaskA());

		// 2.2.时间定时
		// 如果设定时间已过时，也会触发定时器
		m_timer.add_timer("2017-03-13 00:18:10", std::bind(
			&App::task_func, this, 'D', std::placeholders::_1));
		m_timer.add_timer("2017-03-13 00:18:15", TaskA());
		m_timer.add_timer("2017-03-13 00:18:20", std::bind(
			&App::task_func, this, 'D', std::placeholders::_1));
		m_timer.add_timer("2017-03-13 00:18:25", TaskA());

		// 2.3.每日定时
		// 由于获取的时钟精度为秒，所以一旦超过">="该时间点，则定时到下一天。
		// 若使用">"，由于精度为秒，两者时间比较结果为相等，导致反复定时。
		m_timer.add_daily_timer("00:37:50", true, std::bind(
			&App::task_func, this, 'T', std::placeholders::_1));
		m_timer.add_daily_timer("00:37:55", true, TaskA());

		// 结束
		//eco::this_thread::sleep(30000);
		//m_timer.stop();
		//return;
	}

	// #3.生命对象
	// 功能：on_born\on_live触发。
	//g_being.born();
	// 在on_live中析构非自己的生命对象。（正常）
	//g_release_being.born();
	// 在on_live中析构自己的生命对象。（死锁）
	//g_me_being.born();
	// 多线程同时释放生命对象。
	m_thread_pool.run(BeingTest, 20, "being_test");
}


////////////////////////////////////////////////////////////////////////////////
void App::on_exit()
{
	// 定时器
	//m_timer.stop();
	m_thread_pool.join();
}


////////////////////////////////////////////////////////////////////////////////
}}}