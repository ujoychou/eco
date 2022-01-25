#include "Pch.h"
#include "Eco.ipp"
////////////////////////////////////////////////////////////////////////////////
#include <eco/rx/RxImpl.h>
#include <eco/thread/Btask.h>


ECO_NS_BEGIN(eco);
uint32_t Eco::Impl::s_task_thread_size = 0;
////////////////////////////////////////////////////////////////////////////////
Eco::Impl::Impl() {}
uint32_t Eco::Impl::get_task_thread_size()
{
	return s_task_thread_size;
}
void Eco::Impl::set_task_thread_size(IN uint32_t v)
{
	s_task_thread_size = v;
}
void Eco::Impl::start()
{
	// 启动时间轮
	m_wheel.start("eco_wheel");

	// 启动生命活动线程
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
void Eco::Impl::post_task(eco::Task&& task, uint32_t restart_sec_if_fail)
{
	m_task_server.post(std::move(task), restart_sec_if_fail);
}
void Eco::Impl::post_wait(IN Btask::ptr&& task)
{
	std_lock_guard lock(m_wait_task_list_mutex);
	m_wait_task_list.push_back(std::move(task));
}
void Eco::Impl::move_wait()
{
	// 重新检查等待任务是否可执行。
	std_lock_guard lock(m_wait_task_list_mutex);
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
			m_task_server.post(
				std::bind(&Btask::operator(), btask), btask->restart_secs());
		}
		it = m_wait_task_list.erase(it);
	}
}


////////////////////////////////////////////////////////////////////////////////
ECO_SINGLETON_IMPL(Eco);
Eco& Eco::get() { return Singleton<Eco>::get(); }
void Eco::post_task(eco::Task&& task, uint32_t restart_sec_if_fail)
{
	impl().post_task(std::move(task), restart_sec_if_fail);
}
void Eco::post_task(IN Btask::ptr&& task)
{
	impl().post_task(std::bind(&Task::operator(), task), task->restart_secs());
}
void Eco::post_wait(IN std::shared_ptr<Btask>&& task)
{
	impl().post_wait(std::move(task));
}
void Eco::move_wait()
{
	impl().move_wait();
}
eco::Timing& Eco::timer()
{
	return impl().m_wheel;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);