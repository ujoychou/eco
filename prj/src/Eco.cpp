#include "PrecHeader.h"
#include "Eco.ipp"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Being.h>
#include <eco/thread/Thread.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class ThreadCheckOut
{
public:
	inline static void on_timer(ThreadCheck& obj) { obj.on_timer();	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_SINGLETON_IMPL(Eco);
Eco& Eco::get()
{
	return Singleton<Eco>::instance();
}


////////////////////////////////////////////////////////////////////////////////
Eco::Impl::Impl()
{
	m_tick_count = 0;
	reset_current_being();
}
// 默认每1秒1次，检测与修复系统。
uint32_t Eco::Impl::s_unit_live_tick_sec = 1;
uint32_t Eco::Impl::s_task_server_thread_size = 0;


////////////////////////////////////////////////////////////////////////////////
void Eco::Impl::start()
{
	if (s_unit_live_tick_sec == 0)
	{
		s_unit_live_tick_sec = 1;
	}

	// 启动生命活动线程
	m_timer.start();
	if (s_task_server_thread_size > 0)
	{
		m_task_server.run("btask", s_task_server_thread_size);
	}

	// 启动生命节奏：定时执行系统维护工作。
	uint32_t millsecs = s_unit_live_tick_sec * 1000;
	m_timer.add(millsecs, true, std::bind(&Impl::on_live_timer, this));
}


////////////////////////////////////////////////////////////////////////////////
void Eco::Impl::stop()
{
	m_timer.stop();
	if (s_task_server_thread_size > 0)
	{
		m_task_server.stop();
	}
}


////////////////////////////////////////////////////////////////////////////////
void Eco::Impl::on_live_timer()
{
	ThreadCheckOut().on_timer(ThreadCheck::me());

	// 生命对象的周期活动，每个生命节奏时间运行一次。
	++m_tick_count;
	while (Being* be = get_next_being())
	{
		// 生命对象的OnLive方法中，不能调用其析构函数，否则会造成递归死锁。
		if (is_time_to_live(be->get_live_ticks()) && be->to_live())
		{
			try
			{
				be->on_live();			// 2.运行生命对象的活动方法
			}
			catch (eco::Error& e)
			{
				ECO_LOG(error, "live") << be->get_name() <= e;
			}
			catch (std::exception& e)
			{
				ECO_LOGX(error, "live") << be->get_name() <= e.what();
			}
			
			m_running.none();			// 3.生命对象运行结束
			m_cond_var.notify_one();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
Being* Eco::Impl::get_next_being()
{
	eco::Mutex::ScopeLock lock(m_cond_var.mutex());
	if (m_cur_it == m_be_list.end())
	{
		m_cur_it = m_be_list.begin();
	}
	else
	{
		++m_cur_it;
	}
	m_running.ok();		// 1.生命对象运行开始
	return get_current_being();
}


////////////////////////////////////////////////////////////////////////////////
void Eco::Impl::add_being(IN Being* be)
{
	eco::Mutex::ScopeLock lock(m_cond_var.mutex());
	if (be != nullptr)
	{
		m_be_list.push_back(be);
	}
}


////////////////////////////////////////////////////////////////////////////////
bool Eco::Impl::is_time_to_live(IN const uint32_t interval_tick) const
{
	//int interval = secs / m_unit_live_tick_sec;
	return (m_tick_count == 0 || m_tick_count % interval_tick == 0);
}


////////////////////////////////////////////////////////////////////////////////
void Eco::Impl::reset_current_being()
{
	eco::Mutex::ScopeLock lock(m_cond_var.mutex());
	m_cur_it = m_be_list.end();
}


////////////////////////////////////////////////////////////////////////////////
Being* Eco::Impl::get_current_being() const
{
	return m_cur_it != m_be_list.end() ? *m_cur_it : nullptr;
}


////////////////////////////////////////////////////////////////////////////////
void Eco::Impl::remove_being(IN Being* be)
{
	// 支持在on_live方法中释放除自己意外的其他生命对象。
	eco::Mutex::ScopeLock lock(m_cond_var.mutex());

	// 等待当前正在活动的生命对象，必须等待生命对象执行完成后才能删除。
	while (get_current_being() == be && m_running.is_ok())
	{
		m_cond_var.wait();
	}

	// 删除对象列表中的对象。
	auto it = std::find(m_be_list.begin(), m_be_list.end(), be);
	if (it != m_be_list.end())
	{
		bool is_current = (it == m_cur_it);
		it = m_be_list.erase(it);
		if (is_current)		// 更新当前对象位置
		{
			m_cur_it = (it == m_be_list.begin()) ? m_be_list.end() : --it;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
void Eco::Impl::post_task(IN Task::ptr& task)
{
	m_task_server.queue().post(task);
}
void Eco::Impl::post_wait(IN Task::ptr& task)
{
	eco::Mutex::ScopeLock lock(m_wait_task_list_mutex);
	m_wait_task_list.push_back(std::move(task));
}
void Eco::Impl::move_wait()
{
	// 重新检查等待任务是否可执行。
	eco::Mutex::ScopeLock lock(m_wait_task_list_mutex);
	auto it = m_wait_task_list.begin();
	while (it != m_wait_task_list.end())
	{
		Btask* btask = (Btask*)it->get();
		eco::TaskState state = btask->occupy();
		if (state == task_no_ready || state == task_working_other)
		{
			++it;
			continue;
		}
		else if (state == task_occupied)
		{
			m_task_server.post(*it);
		}
		it = m_wait_task_list.erase(it);
	}
}


////////////////////////////////////////////////////////////////////////////////
void Eco::post_task(IN Task::ptr& task)
{
	impl().post_task(task);
}
void Eco::post_wait(IN Task::ptr& task)
{
	impl().post_wait(task);
}
void Eco::move_wait()
{
	impl().move_wait();
}
TimerServer& Eco::timer()
{
	return impl().m_timer;
}


////////////////////////////////////////////////////////////////////////////////
uint32_t Eco::post_async(IN std::shared_ptr<net::MessageHandler>& hdl)
{
	// note: request != 0.
	if (++impl().m_request_id == 0)	++impl().m_request_id;
	uint32_t req_id = impl().m_request_id;
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
std::shared_ptr<net::MessageHandler> Eco::pop_async(
	IN uint32_t req_id, IN bool last)
{
	std::shared_ptr<net::MessageHandler> handler;
	if (last)
	{
		int eid = 0;
		handler = impl().m_async_map.pop(req_id, eid);
	}
	else
	{
		impl().m_async_map.find(handler, req_id);
	}
	return handler;
}


////////////////////////////////////////////////////////////////////////////////
uint32_t Eco::Impl::get_unit_live_tick_seconds()
{
	return s_unit_live_tick_sec;
}
void Eco::Impl::set_unit_live_tick_seconds(IN uint32_t v)
{
	s_unit_live_tick_sec = v;
	if (s_unit_live_tick_sec == 0)
	{
		s_unit_live_tick_sec = 1;
	}
}
uint32_t Eco::Impl::get_task_server_thread_size()
{
	return s_task_server_thread_size;
}
void Eco::Impl::set_task_server_thread_size(IN uint32_t v)
{
	s_task_server_thread_size = v;
}
TimerServer& Eco::Impl::timer()
{
	return m_timer;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);