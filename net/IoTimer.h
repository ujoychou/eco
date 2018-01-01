#ifndef ECO_NET_IO_TIMER_H
#define ECO_NET_IO_TIMER_H
/*******************************************************************************
@ name


@ function


@ exception


@ remark


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-12.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Project.h>



namespace eco{;
namespace net{;


class IoService;
typedef std::function<void(IN const eco::Error* error)> OnTimer;
////////////////////////////////////////////////////////////////////////////////
class IoTimer
{
	ECO_MOVABLE_API(IoTimer);
public:
	// register "on_timer" event handler.
	void register_on_timer(IN OnTimer handler);

	// set io service that running timer depends on.
	void set_io_service(IN IoService& srv);

	// start timer.
	void set_timer(IN uint32_t tick_secs);

	// cancel timer.
	size_t cancel();
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif