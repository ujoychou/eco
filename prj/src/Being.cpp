#include "PrecHeader.h"
#include <eco/Being.h>
////////////////////////////////////////////////////////////////////////////////
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
	// �������Ƶ�ʡ�������Ƶ�ʣ�
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
	kill();
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
	// �������������̰߳�ȫ�ԣ�
	if (!impl().m_born.is_ok())
	{
		if (get_eco() == nullptr)
		{
			EcoThrow << "there is no eco life when live " << impl().m_name;
			return;
		}

		if (on_born())	// ������ʼ��
		{
			get_eco()->add_being(this);
			impl().m_born.ok();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
void Being::live()
{
	// ��������������̰߳�ȫ�ԣ�
	if (!impl().m_born.is_ok())
	{
		if (get_eco() == nullptr)
		{
			EcoThrow << "there is no eco life when live " << impl().m_name;
			return;
		}

		if (on_born())
		{
			try
			{
				on_live();
			}
			catch (eco::Error& e)
			{
				EcoError << get_name() << " live: " << e;
			}
			catch (std::exception& e) 
			{
				EcoError << get_name() << " live: " << e.what();
			}
			
			get_eco()->add_being(this);
			impl().m_born.ok();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
void Being::kill()
{
	if (impl().m_born.is_ok())
	{
		get_eco()->remove_being(this);
		impl().m_born.none();
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