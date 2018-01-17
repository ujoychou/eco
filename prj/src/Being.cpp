#include "PrecHeader.h"
#include <eco/Being.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/log/Log.h>
#include <eco/thread/State.h>
#include <eco/Implement.h>
#include "Eco.h"


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
class Being::Impl
{
	ECO_IMPL_INIT(Being);
public:
	// 生命活动的频率。（心跳频率）
	uint32_t m_live_ticks;
	eco::atomic::State m_born;
	std::string m_name;
};


////////////////////////////////////////////////////////////////////////////////
ECO_IMPL(Being);
ECO_PROPERTY_STR_IMPL(Being, name);
Being::Being(IN const uint32_t live_ticks)
{
	m_impl = new Impl();
	impl().m_live_ticks = live_ticks > 0 ? live_ticks : 1;
}
Being::~Being()
{
	if (impl().m_born.is_ok())
	{
		get_eco()->remove_being(this);
		impl().m_born.none();
	}
}


////////////////////////////////////////////////////////////////////////////////
bool Being::is_born() const
{
	return impl().m_born.is_ok();
}
const uint32_t Being::get_live_ticks() const
{
	return impl().m_live_ticks;
}
void Being::set_live_ticks(IN const uint32_t ticks)
{
	impl().m_live_ticks = ticks;
}


////////////////////////////////////////////////////////////////////////////////
void Being::born()
{
	if (!impl().m_born.is_ok())
	{
		if (on_born())	// 生命初始化
		{
			get_eco()->add_being(this);
			impl().m_born.ok();
		}
	}	
}


////////////////////////////////////////////////////////////////////////////////
void Being::live()
{
	// 开启生命活动
	if (!impl().m_born.is_ok())
	{
		if (on_born())
		{
			try
			{
				on_live();
			}
			catch (std::exception& e) 
			{
				EcoError << impl().m_name << " live: " << e.what();
			}
			// 避免多个线程同时访问"on_live()";
			get_eco()->add_being(this);
			impl().m_born.ok();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
const uint32_t Being::get_unit_live_tick_seconds()
{
	return get_eco()->get_unit_live_tick_seconds();
}
void Being::post_task(IN Btask& task)
{
	get_eco()->post_task(task);
}

////////////////////////////////////////////////////////////////////////////////
}