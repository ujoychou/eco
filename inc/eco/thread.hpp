#pragma once
/*******************************************************************************
@ name
thread and common thread function.

@ function


@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy created on 2025-08-14.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2025 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/cast.hpp>
#include <eco/export/api.hpp>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
class eco_api thread
{
public:
	// count thread size.
	static uint32_t count();

	// get thread id;
	uint32_t id() const;
	const char* sid() const;

	// thread name.
	const char* name() const;

	// waiting thread over.
	void join();
};

////////////////////////////////////////////////////////////////////////////////
eco_namespace(this_thread);
// get current thread id.
eco_api uint32_t id();
eco_api const char* sid();

// get current thread name.
eco_api const char* name();

eco_api void yield();

eco_api void sleep(int ms);

// init thread id.
eco_api void init();
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(this_thread);
eco_namespace_end(eco);