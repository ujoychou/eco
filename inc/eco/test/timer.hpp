#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2025-06-25] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/macro.hpp>
#include <chrono>


eco_namespace(eco);
eco_namespace(test);
////////////////////////////////////////////////////////////////////////////////
class timer
{
public:
	inline timer()
	{
		m_clock = std::chrono::steady_clock::now();
	}

	inline void start()
	{
		m_clock = std::chrono::steady_clock::now();
	}

	inline int64_t timeup(eco::bool_t start_next_timer = true)
	{
		auto cur = std::chrono::steady_clock::now();
		auto dur = std::chrono::duration_cast<
			std::chrono::milliseconds>(cur - m_clock);
		return dur.count();
	}

private:
	std::chrono::steady_clock::time_point m_clock;
};


////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(test);
eco_namespace_end(eco);