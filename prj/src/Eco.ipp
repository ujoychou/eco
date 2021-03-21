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
#include <eco/Eco.h>
#include <eco/thread/Map.h>
#include <eco/thread/State.h>
#include <eco/thread/TaskServer.h>
#include <eco/thread/TimingWheel.h>
#include <list>


ECO_NS_BEGIN(eco);
class Being;
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
	void post_task(IN Closure&& task);

	// 加入等待任务
	void post_wait(IN Btask::ptr& task);

	// 通知等待任务
	void move_wait();

	// 任务服务线程
	static void set_task_thread_size(uint32_t size);
	static uint32_t get_task_thread_size();

	// get next req id.
	inline uint32_t next_req_id()
	{
		std_lock_guard lock(m_async_map.mutex());
		// promise that: request_id != 0.
		if (++m_request_id == 0) ++m_request_id;
		return m_request_id;
	}

public:
	// 时间轮定时器
	eco::TimingWheel m_wheel;

	// 任务队列
	static uint32_t s_task_thread_size;
	eco::TaskServer m_task_server;
	std::list<Btask::ptr> m_wait_task_list;
	mutable eco::Mutex m_wait_task_list_mutex;

	// 服务异步管理
	eco::Atomic<uint32_t> m_request_id;
	eco::HashMap<uint32_t, Eco::HandlerPtr> m_async_map;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif