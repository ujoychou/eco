#ifndef ECO_TIMING_H
#define ECO_TIMING_H
/*******************************************************************************
@ name
timing.

@ function
1.use to calculate the program runing time.


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-08-15.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>


namespace eco{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
// timer for task.
class ECO_API Timing
{
	ECO_OBJECT_API(Timing);
public:
	// start timer.
	void start();

	// time is up, and it will restart timer.
	Timing& timeup();

	// get interval from timer start to the timeup point.
	int64_t seconds() const;
	int64_t milliseconds() const;
	int64_t microseconds() const;
};


////////////////////////////////////////////////////////////////////////////////
}}// ns
#endif