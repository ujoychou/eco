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


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class ECO_API Being
{
	ECO_IMPL_API();
	ECO_OBJECT(Being);
public:
	// 生命对象单位生命活动频率
	static const uint32_t get_unit_live_tick_seconds();

	// 构建生命。默认时间 = 10 seconds.
	Being(IN uint32_t live_secs = 10, IN const char* name = "");

	// 析构生命
	virtual ~Being();

	// 开始生命
	void born();

	// 结束生命
	void kill();

	// 开始生命活动
	void live();

	// 生命是否已开始
	bool is_born() const;

	// 生命活动频率：按Tick或按Seconds。
	void set_live_ticks(IN uint32_t ticks);
	uint32_t get_live_ticks() const;
	void set_live_seconds(IN uint32_t secs);
	uint32_t get_live_seconds() const;

	// 生命活动频率：实际运行时间
	uint32_t get_living_seconds() const;

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

	// 维系生命：活动修复
	virtual void on_live()
	{}

	// 是否运行活动方法
	virtual bool to_live()
	{
		return true;
	}

	friend class Eco;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif