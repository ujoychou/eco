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
	// ���췽��
	Impl();

	// ���л
	void start();

	// ֹͣ�
	void stop();

	// ����ִ������
	void post_task(IN Closure&& task);

	// ����ȴ�����
	void post_wait(IN Btask::ptr& task);

	// ֪ͨ�ȴ�����
	void move_wait();

	// ��������߳�
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
	// ʱ���ֶ�ʱ��
	eco::TimingWheel m_wheel;

	// �������
	static uint32_t s_task_thread_size;
	eco::TaskServer m_task_server;
	std::list<Btask::ptr> m_wait_task_list;
	mutable eco::Mutex m_wait_task_list_mutex;

	// �����첽����
	eco::Atomic<uint32_t> m_request_id;
	eco::HashMap<uint32_t, Eco::HandlerPtr> m_async_map;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif