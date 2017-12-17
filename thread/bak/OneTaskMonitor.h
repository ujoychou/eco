#ifndef ECO_THREAD_ONE_TASK_MONITOR_H
#define ECO_THREAD_ONE_TASK_MONITOR_H
/*******************************************************************************
@ name

@ function
1.apply scene: client api(like ctp\esunny) control request once a time.
2.apply scene: all people just finish a task, who ocuppy first who do it.

@ exception

@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2017 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Project.h>
#include <eco/atomic/Atomic.h>
#include <eco/thread/Mutex.h>
#include <eco/thread/CondVar.h>
#include <eco/thread/TaskServer.h>


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
class ExclusiveWorker
{
public:
	inline ExclusiveWorker() : m_busy(false)
	{}

	inline bool busy() const
	{
		return (m_busy == true);
	}

	inline bool occupy()
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		if (busy())
		{
			return false;
		}
		m_busy = true;
		return true;
	}

	inline void release()
	{
		m_busy = false;
	}

protected:
	eco::Mutex m_mutex;
	eco::Atomic<uint32_t> m_busy;
};


////////////////////////////////////////////////////////////////////////////////
class ExclusiveMonitor : public ExclusiveWorker
{
public:
	inline ExclusiveMonitor() : m_busy(false)
	{}

	inline void wait_occupy()
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		while (busy())
		{
			m_cond_var.wait();
		}
		m_busy = true;
	}

	inline bool timed_occupy(IN uint32_t millsec)
	{
		eco::Mutex::ScopeLock lock(m_mutex);
		while (busy())
		{
			if (!m_cond_var.timed_wait(millsec))
			{
				return false;
			}
		}
		m_busy = true;
		return true;
	}

	inline void release()
	{
		ExclusiveWorker::release();
		m_cond_var.notify_one();
	}

	eco::CondVar m_cond_var;
};


////////////////////////////////////////////////////////////////////////////////
class ExclusiveWorkerInit
{
public:
	inline ExclusiveWorkerInit(
		IN ExclusiveWorker& worker)
		: m_worker(worker)
		, m_worker_occupy(false)
		, m_task_state(false)
	{}

	inline ~ExclusiveWorkerInit()
	{
		if (m_worker_occupy && !m_task_state)
		{
			m_worker.release();
		}
	}

	// occupy the task.
	inline bool occupy()
	{
		return m_worker_occupy = m_worker.occupy();
	}

	// finish the task.
	inline void set_task_state(IN bool is_ok = true)
	{
		m_task_state = is_ok;
	}

private:
	ExclusiveWorker& m_worker;
	uint16_t m_worker_occupy;
	uint16_t m_task_state;
};


////////////////////////////////////////////////////////////////////////////////
class ExclusiveWorkerExit
{
public:
	inline ExclusiveWorkerExit(
		IN ExclusiveWorker& worker,
		IN uint32_t is_exit = true)
		: m_worker(worker)
		, m_is_exit(is_exit)
	{}

	inline ~ExclusiveWorkerExit()
	{
		if (m_is_exit)
		{
			m_worker.release();
		}
	}

private:
	ExclusiveWorker& m_worker;
	uint32_t m_is_exit;
};


////////////////////////////////////////////////////////////////////////////////
class CooperativeTask
{
public:
};



////////////////////////////////////////////////////////////////////////////////
#include <eco/Repository.h>
#include <eco/DateTime.h>

class TaskSubject
{
public:
	inline const eco::Mutex& mutex() const
	{
		return m_mutex;
	}

	inline const bool finished() const
	{
		return m_state.ok();
	}

private:
	mutable eco::Mutex m_mutex;
	eco::atomic::State m_state;
};


// 账户列表
std::vector<std::string> m_account_set;

// 合约信息
class ContractSet
{
	ECO_OBJECT(ContractSet);
public:
	TaskSubject m_query_task;
	std::string m_timestamp;
	eco::atomic::State m_state;
};

// 合约保证金率
class MarginRateSet
{
	ECO_OBJECT(MarginRateSet);
public:
};

// 合约手续费率
class CommissionRateSet
{
	ECO_OBJECT(CommissionRateSet);
public:
};

// 账户合约信息
ContractSet::ptr m_contract_set;

eco::Repository<std::string, MarginRateSet::ptr> m_account_margin_map;
eco::Repository<std::string, CommissionRateSet::ptr> m_account_comm_map;

// 合约信息查询者
class Queryer
{
	ECO_OBJECT(Queryer);
public:
};
eco::Repository<std::string, Queryer::ptr> m_account_queryer_map;


////////////////////////////////////////////////////////////////////////////////
/* 查询合约\保证金率\手续费率：
1.周期性任务，每天20:00结算后开始查询。
*/
class Task : public eco::Object<Task>
{
protected:
	virtual void reset() = 0;
	virtual void execute() = 0;
	virtual void subject() = 0;

public:
	inline Task(
		IN TaskSubject& subject,
		IN bool restart_task = false)
		: m_restart_task(restart_task)
		, m_task_subject(subject)
	{}

	inline bool restart() const
	{
		return m_restart_task > 0;
	}

	inline bool finished() const
	{
		return m_task_subject.finished();
	}

	inline void operator()(void)
	{
		eco::Mutex::ScopeLock lock(m_task_subject.mutex());

		// #1.task hasn't been finished.
		if (!m_task_subject.finished())
		{
			while (!execute_sufficient())
			{
				if (execute_sufficient_wait())
				{
					break;
				}
			}
			execute();
			return;
		}

		// #2.task has been finished.
		if (m_restart_task)
		{
			reset();
			if (!m_task_subject.finished())
			{
				execute();
			}
		}
	}

	inline bool execute_sufficient()
	{
		auto it = m_sufficient_tasks.begin();
		for (; it != m_sufficient_tasks.end(); ++it)
		{
			(**it)();
			if (!(**it).finished())
			{
				return false;
			}
		}
		return true;
	}

	inline bool execute_sufficient_wait()
	{
		// 充分任务正在被执行，则等待。

		// 充分任务执行完成，执行当前任务。
		if ()
		{
		}
	}

protected:
	uint16_t m_restart_task;
	TaskSubject& m_task_subject;
	std::vector<Task::ptr> m_sufficient_tasks;
};


class QueryContractTask : public Task
{
public:
	inline void operator()(void)
	{
		eco::date_time::Timestamp ts;
		if (ts == m_contract_set->m_timestamp)
		{
		}
	}

private:

};


class QueryMarginRateTask : public TaskObject
{
public:
	inline void operator()(void)
	{
	}

private:
	
};


class QueryCommissionRateTask : public TaskObject
{
public:
};


class App
{
public:
	void on_init()
	{
		// 初始化账户列表
		init_account_set(m_account_set);

		// 多个线程同时查询
		eco::TaskServer server;
		server.run(m_account_set.size());
	}

	void on_daily_query()
	{
		// 查询合约
		server.post();

		// 查询保证金率

		// 查询手续费率
	}
};


void init_account_set(OUT std::vector<std::string>&);
void main()
{
}



////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif