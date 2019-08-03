#include "PrecHeader.h"
#include <eco/Being.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/log/Log.h>
#include <eco/thread/State.h>
#include <eco/Implement.h>
#include <Eco.ipp>


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
class Being::Impl
{
	ECO_IMPL_INIT(Being);
public:
	// 生命活动的频率。（心跳频率）
	std::string m_name;
	uint32_t m_live_ticks;
	uint32_t m_live_seconds;
	eco::atomic::State m_born;
	
	inline Impl() : m_live_ticks(0), m_live_seconds(0)
	{}

	inline void set_live_seconds(IN const uint32_t secs)
	{
		if (secs > 0)
		{
			m_live_ticks = secs / get_unit_live_tick_seconds();
			if (secs - m_live_ticks * get_unit_live_tick_seconds() > 0)
			{
				++m_live_ticks;
			}
			return;
		}
		m_live_ticks = 0;
	}

	inline bool on_born(Being& be)
	{
		if (m_live_seconds > 0)
		{
			set_live_seconds(m_live_seconds);	// 设置心跳频率
		}
		return be.on_born();	// 生命初始化
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_IMPL(Being);
ECO_PROPERTY_STR_IMPL(Being, name);
Being::Being(IN uint32_t live_sec, IN const char* name)
{
	m_impl = new Impl();
	set_live_seconds(live_sec);
	m_impl->m_name = name;
}
Being::~Being()
{
	kill();
	delete m_impl;
	m_impl = nullptr;
}


////////////////////////////////////////////////////////////////////////////////
bool Being::is_born() const
{
	return impl().m_born.is_ok();
}
uint32_t Being::get_live_ticks() const
{
	return impl().m_live_ticks;
}
void Being::set_live_ticks(IN uint32_t ticks)
{
	impl().m_live_ticks = ticks;
}
void Being::set_live_seconds(IN uint32_t secs)
{
	impl().m_live_seconds = secs;
	impl().set_live_seconds(secs);
}
uint32_t Being::get_live_seconds() const
{
	return impl().m_live_seconds;
}
uint32_t Being::get_living_seconds() const
{
	return impl().m_live_ticks * Eco::Impl::get_unit_live_tick_seconds();
}


////////////////////////////////////////////////////////////////////////////////
void Being::born()
{
	// 生命降生（非线程安全性）
	if (!impl().m_born.is_ok())
	{
		if (impl().on_born(*this))
		{
			eco().impl().add_being(this);
			impl().m_born.ok();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
void Being::live()
{
	// 开启生命活动（非线程安全性）
	if (!impl().m_born.is_ok())
	{
		if (impl().on_born(*this))
		{
			try
			{
				on_live();
			}
			catch (eco::Error& e)
			{
				ECO_ERROR << get_name() << " live : " << e;
			}
			catch (std::exception& e) 
			{
				ECO_ERROR << get_name() << " live : " << e.what();
			}
			
			eco().impl().add_being(this);
			impl().m_born.ok();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
void Being::kill()
{
	if (impl().m_born.is_ok())
	{
		eco().impl().remove_being(this);
		impl().m_born.none();
	}
}


////////////////////////////////////////////////////////////////////////////////
const uint32_t Being::get_unit_live_tick_seconds()
{
	return Eco::Impl::get_unit_live_tick_seconds();
}


////////////////////////////////////////////////////////////////////////////////
}