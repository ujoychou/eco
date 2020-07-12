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
#include <eco/thread/State.h>
#include <eco/thread/TaskServer.h>
#include <eco/thread/Timer.h>
#include <eco/thread/Map.h>
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
	void post_task(IN Task::ptr& task);

	// 加入等待任务
	void post_wait(IN Task::ptr& task);

	// 通知等待任务
	void move_wait();

	// 添加生命对象
	void add_being(IN Being* be);
	// 移除生命对象
	void remove_being(IN Being* be);

	// 定时器
	TimerServer& timer();

	// 生命活动频率
	static void set_unit_live_tick_seconds(uint32_t secs);
	static uint32_t get_unit_live_tick_seconds();

	// 任务服务线程
	static void set_task_server_thread_size(uint32_t size);
	static uint32_t get_task_server_thread_size();
	
private:
	// 生命节奏响应方法：执行检测与修复系统生命对象。
	// 开始生命节奏。（类似于开始心跳）
	void start_live_timer();
	void on_live_timer();
	bool is_time_to_live(IN const uint32_t interval_tick) const;
	// 获取下一个将被执行的活动。
	Being* get_next_being();
	// 当前生命对象
	void reset_current_being();
	Being* get_current_being() const;

public:
	// 系统生命：生命节奏（每x秒1次），用于维系系统正常运行。
	static uint32_t s_unit_live_tick_sec;
	uint32_t m_tick_count;
	eco::TimerServer m_timer;

	// 生命对象列表
	std::list<Being*> m_be_list;
	// 当前正在运行的生命对象
	std::list<Being*>::iterator m_cur_it;
	eco::atomic::State m_running;
	mutable eco::ConditionVariable m_cond_var;

	// 任务队列
	static uint32_t s_task_server_thread_size;
	eco::TaskServer m_task_server;
	std::list<Task::ptr> m_wait_task_list;
	mutable eco::Mutex m_wait_task_list_mutex;

	// 服务异步管理
	mutable eco::Mutex m_mutex;
	eco::Atomic<uint32_t> m_request_id;
	eco::HashMap<uint32_t, std::shared_ptr<net::MessageHandler> > m_async_map;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif