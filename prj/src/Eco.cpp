#include "PrecHeader.h"
#include "Eco.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Being.h>
#include <eco/Btask.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
Eco::Eco()
{
	m_tick_count = 0;
	m_unit_live_tick_sec = 5;		// Ĭ��ÿ5��1�Σ�������޸�ϵͳ��
	m_task_server_thread_size = 2;
	reset_current_being();
}


////////////////////////////////////////////////////////////////////////////////
Eco::~Eco()
{}


////////////////////////////////////////////////////////////////////////////////
void Eco::start()
{
	if (m_unit_live_tick_sec == 0)
	{
		m_unit_live_tick_sec = 1;
	}

	// ����������߳�
	m_timer.start();
	m_task_server.run(m_task_server_thread_size, "eco_task");

	// �����������ࣺ��ʱִ��ϵͳά��������
	uint32_t millsecs = m_unit_live_tick_sec * 1000;
	m_timer.add_timer(millsecs, true, std::bind(&Eco::on_live_timer, this));
}


////////////////////////////////////////////////////////////////////////////////
void Eco::stop()
{
	m_timer.stop();
	m_task_server.stop();
}


////////////////////////////////////////////////////////////////////////////////
void Eco::on_live_timer()
{
	// ������������ڻ��ÿ����������ʱ������һ�Ρ�
	++m_tick_count;
	while (Being* be = get_next_being())
	{
		// ���������OnLive�����У����ܵ����������������������ɵݹ�������
		if (is_time_to_live(be->get_live_ticks()) && be->to_live())
		{
			try
			{
				be->on_live();			// 2.������������Ļ����
			}
			catch (eco::Error& e)
			{
				EcoError << be->get_name() << "live : " << e;
			}
			catch (std::exception& e)
			{
				EcoError << be->get_name() << "live : " << e.what();
			}
			
			m_running.none();			// 3.�����������н���
			m_cond_var.notify_one();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
Being* Eco::get_next_being()
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
	m_running.ok();		// 1.�����������п�ʼ
	return get_current_being();
}


////////////////////////////////////////////////////////////////////////////////
void Eco::add_being(IN Being* be)
{
	eco::Mutex::ScopeLock lock(m_cond_var.mutex());
	if (be != nullptr)
	{
		m_be_list.push_back(be);
	}
}


////////////////////////////////////////////////////////////////////////////////
bool Eco::is_time_to_live(IN const uint32_t interval_tick) const
{
	//int interval = secs / m_unit_live_tick_sec;
	return (m_tick_count == 0 || m_tick_count % interval_tick == 0);
}


////////////////////////////////////////////////////////////////////////////////
void Eco::reset_current_being()
{
	eco::Mutex::ScopeLock lock(m_cond_var.mutex());
	m_cur_it = m_be_list.end();
}


////////////////////////////////////////////////////////////////////////////////
Being* Eco::get_current_being() const
{
	return m_cur_it != m_be_list.end() ? *m_cur_it : nullptr;
}


////////////////////////////////////////////////////////////////////////////////
void Eco::remove_being(IN Being* be)
{
	// ֧����on_live�������ͷų��Լ������������������
	eco::Mutex::ScopeLock lock(m_cond_var.mutex());

	// �ȴ���ǰ���ڻ���������󣬱���ȴ���������ִ����ɺ����ɾ����
	while (get_current_being() == be && m_running.is_ok())
	{
		m_cond_var.wait();
	}

	// ɾ�������б��еĶ���
	auto it = std::find(m_be_list.begin(), m_be_list.end(), be);
	if (it != m_be_list.end())
	{
		bool is_current = (it == m_cur_it);
		it = m_be_list.erase(it);
		if (is_current)		// ���µ�ǰ����λ��
		{
			m_cur_it = (it == m_be_list.begin()) ? m_be_list.end() : --it;
		}
	}
}
void Eco::post_task(IN const Btask& task)
{
	std::auto_ptr<Btask> ap(task.copy());
	m_task_server.post(std::move(ap));
}
void Eco::post_wait(IN const Btask& task)
{
	std::auto_ptr<Btask> ap(task.copy());
	eco::Mutex::ScopeLock lock(m_wait_task_list_mutex);
	m_wait_task_list.push_back(std::move(ap));
}
void Eco::move_wait()
{
	// ���¼��ȴ������Ƿ��ִ�С�
	eco::Mutex::ScopeLock lock(m_wait_task_list_mutex);
	auto it = m_wait_task_list.begin();
	while (it != m_wait_task_list.end())
	{
		eco::TaskState state = it->get()->occupy();
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
const uint32_t Eco::get_unit_live_tick_seconds() const
{
	return m_unit_live_tick_sec;
}
void Eco::set_unit_live_tick_seconds(IN const uint32_t unit_live_tick_secs)
{
	m_unit_live_tick_sec = unit_live_tick_secs;
	if (m_unit_live_tick_sec == 0)
	{
		m_unit_live_tick_sec = 1;
	}
}
const uint32_t Eco::get_task_server_thread_size() const
{
	return m_task_server_thread_size;
}
void Eco::set_task_server_thread_size(IN const uint32_t thread_size)
{
	m_task_server_thread_size = thread_size;
	if (m_task_server_thread_size == 0)
	{
		m_task_server_thread_size = 1;
	}
}
Timer& Eco::timer()
{
	return m_timer;
}


////////////////////////////////////////////////////////////////////////////////
static std::auto_ptr<Eco> s_eco;
void create_eco()
{
	s_eco.reset(new Eco);
}
Eco* get_eco()
{
	return s_eco.get();
}


////////////////////////////////////////////////////////////////////////////////
void Btask::post_task(IN const Btask& task)
{
	get_eco()->post_task(task);
}
void Btask::post_wait(IN const Btask& task)
{
	get_eco()->post_wait(task);
}
void Btask::move_wait()
{
	get_eco()->move_wait();
}
void Btask::set_timer(IN const uint32_t restart_millsecs, IN const Btask& task)
{
	get_eco()->timer().add_timer(restart_millsecs, false, task);
}
const eco::TaskState Btask::occupy()
{
	return bobject().occupy(get_type(), 
		m_sour_state, m_dest_state, m_erase_state);
}
void Btask::prepare()
{}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);