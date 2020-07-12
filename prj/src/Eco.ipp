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
	// ���췽��
	Impl();

	// ���л
	void start();

	// ֹͣ�
	void stop();

	// ����ִ������
	void post_task(IN Task::ptr& task);

	// ����ȴ�����
	void post_wait(IN Task::ptr& task);

	// ֪ͨ�ȴ�����
	void move_wait();

	// �����������
	void add_being(IN Being* be);
	// �Ƴ���������
	void remove_being(IN Being* be);

	// ��ʱ��
	TimerServer& timer();

	// �����Ƶ��
	static void set_unit_live_tick_seconds(uint32_t secs);
	static uint32_t get_unit_live_tick_seconds();

	// ��������߳�
	static void set_task_server_thread_size(uint32_t size);
	static uint32_t get_task_server_thread_size();
	
private:
	// ����������Ӧ������ִ�м�����޸�ϵͳ��������
	// ��ʼ�������ࡣ�������ڿ�ʼ������
	void start_live_timer();
	void on_live_timer();
	bool is_time_to_live(IN const uint32_t interval_tick) const;
	// ��ȡ��һ������ִ�еĻ��
	Being* get_next_being();
	// ��ǰ��������
	void reset_current_being();
	Being* get_current_being() const;

public:
	// ϵͳ�������������ࣨÿx��1�Σ�������άϵϵͳ�������С�
	static uint32_t s_unit_live_tick_sec;
	uint32_t m_tick_count;
	eco::TimerServer m_timer;

	// ���������б�
	std::list<Being*> m_be_list;
	// ��ǰ�������е���������
	std::list<Being*>::iterator m_cur_it;
	eco::atomic::State m_running;
	mutable eco::ConditionVariable m_cond_var;

	// �������
	static uint32_t s_task_server_thread_size;
	eco::TaskServer m_task_server;
	std::list<Task::ptr> m_wait_task_list;
	mutable eco::Mutex m_wait_task_list_mutex;

	// �����첽����
	mutable eco::Mutex m_mutex;
	eco::Atomic<uint32_t> m_request_id;
	eco::HashMap<uint32_t, std::shared_ptr<net::MessageHandler> > m_async_map;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif