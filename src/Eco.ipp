#ifndef ECO_ECO_IPP
#define ECO_ECO_IPP
/*******************************************************************************
@ name
@ function
--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2017-01-01.
1.create and init this class.
--------------------------------------------------------------------------------
* copyright(c) 2017 - 2019, ujoy, reserved all right.
*******************************************************************************/
#include <eco/thread/Eco.h>
#include <eco/thread/Map.h>
#include <eco/thread/State.h>
#include <eco/thread/Btask.h>
#include <eco/thread/Tasker.h>
#include <eco/thread/Timing.h>
#include <list>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class Eco::Impl
{
	ECO_IMPL_INIT(Eco);
public:
	// 构造方法
	Impl();

	// 运行活动
	void start();

	// 停止活动
	void stop();

	// 加入执行任务
	void post_task(eco::Task&& task, uint32_t restart_sec_if_fail);

	// 加入等待任务
	void post_wait(Btask::ptr&& task);

	// 通知等待任务
	void move_wait();

	// 任务服务线程
	static void set_task_thread_size(uint32_t size);
	static uint32_t get_task_thread_size();

public:
	// 时间轮定时器
	eco::Timing m_wheel;

	// 任务队列
	static uint32_t s_task_thread_size;
	eco::Tasker m_task_server;
	std::list<Btask::ptr> m_wait_task_list;
	mutable std_mutex m_wait_task_list_mutex;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif