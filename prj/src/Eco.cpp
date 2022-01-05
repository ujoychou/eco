#include "Pch.h"
#include "Eco.ipp"
////////////////////////////////////////////////////////////////////////////////


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
ECO_SINGLETON_IMPL(Eco);
Eco& Eco::get()
{
	return Singleton<Eco>::get();
}
Eco::Impl::Impl()
{}
// Ĭ��ÿ1��1�Σ�������޸�ϵͳ��
uint32_t Eco::Impl::s_task_thread_size = 0;


////////////////////////////////////////////////////////////////////////////////
void Eco::Impl::start()
{
	// ����ʱ����
	m_wheel.start("eco_wheel");

	// ����������߳�
	if (s_task_thread_size > 0)
	{
		m_task_server.run("btask", s_task_thread_size);
	}
}
void Eco::Impl::stop()
{
	m_wheel.stop();
	if (s_task_thread_size > 0)
	{
		m_task_server.stop();
	}
}


////////////////////////////////////////////////////////////////////////////////
void Eco::Impl::post_task(IN Closure&& task)
{
	m_task_server.queue().post(std::move(task));
}
void Eco::Impl::post_wait(IN Btask::ptr& task)
{
	eco::Mutex::ScopeLock lock(m_wait_task_list_mutex);
	m_wait_task_list.push_back(std::move(task));
}
void Eco::Impl::move_wait()
{
	// ���¼��ȴ������Ƿ��ִ�С�
	eco::Mutex::ScopeLock lock(m_wait_task_list_mutex);
	for (auto it = m_wait_task_list.begin(); it != m_wait_task_list.end(); )
	{
		Btask* btask = it->get();
		eco::TaskState state = btask->occupy();
		if (state == task_no_ready || state == task_working_other)
		{
			++it;
			continue;
		}
		else if (state == task_occupied)
		{
			m_task_server.post(std::bind(&Btask::operator(), btask));
		}
		it = m_wait_task_list.erase(it);
	}
}


////////////////////////////////////////////////////////////////////////////////
void Eco::post_task(IN Closure&& task)
{
	impl().post_task(std::move(task));
}
void Eco::post_task(IN Task::ptr& task)
{
	impl().post_task(std::bind(&Task::operator(), task));
}
void Eco::post_task(IN Btask::ptr& task)
{
	impl().post_task(std::bind(&Btask::operator(), task));
}
void Eco::post_wait(IN std::shared_ptr<Btask>&& task)
{
	impl().post_wait(task);
}
void Eco::move_wait()
{
	impl().move_wait();
}
eco::TimingWheel& Eco::timer()
{
	return impl().m_wheel;
}
uint32_t Eco::Impl::get_task_thread_size()
{
	return s_task_thread_size;
}
void Eco::Impl::set_task_thread_size(IN uint32_t v)
{
	s_task_thread_size = v;
}


////////////////////////////////////////////////////////////////////////////////
uint32_t Eco::post_async(IN HandlerPtr& hdl)
{
	// note: request_id != 0.
	uint32_t req_id = impl().next_req_id();
	impl().m_async_map.set(req_id, hdl);
	return req_id;
}
bool Eco::has_async(IN uint32_t req_id)
{
	return impl().m_async_map.has(req_id);
}
void Eco::erase_async(IN uint32_t req_id)
{
	impl().m_async_map.erase(req_id);
}
Eco::HandlerPtr Eco::pop_async(uint32_t req_id, bool last)
{
	int eid = 0;
	if (last) return impl().m_async_map.pop(req_id, eid);

	HandlerPtr handler;
	impl().m_async_map.find(handler, req_id);
	return handler;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);