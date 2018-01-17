#ifndef ECO_BEING_H
#define ECO_BEING_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/Object.h>
#include <eco/Btask.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class ECO_API Being
{
	ECO_IMPL_API();
	ECO_OBJECT(Being);
public:
	// 生命对象单位生命活动频率
	static const uint32_t get_unit_live_tick_seconds();

	// 任务队列
	static void post_task(IN Btask& task);

public:
	// 构建生命。默认时间 = live_ticks * unit_live_tick_seconds = 30s.
	Being(IN uint32_t live_ticks = 6);

	// 析构生命
	virtual ~Being();

	// 开始生命
	void born();

	// 开始生命活动
	void live();

	// 生命是否已开始
	bool is_born() const;

	// 生命活动频率
	void set_live_ticks(IN const uint32_t ticks);
	const uint32_t get_live_ticks() const;

	// 生命体名称
	void set_name(IN const char*);
	const char* get_name() const;
	Being& name(IN const char*);

protected:
	// 创建生命：初始化
	virtual bool on_born()
	{
		return true;
	}

	// 维系生命
	virtual void on_live()
	{}

	friend class Eco;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif