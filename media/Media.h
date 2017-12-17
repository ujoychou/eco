#ifndef ECO_MEDIA_H
#define ECO_MEDIA_H
/*******************************************************************************
@ name
media.

@ function
1.play sound.

@ exception

@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>



namespace eco{;

////////////////////////////////////////////////////////////////////////////////
enum
{
	play_mode_none			= 0x00000000,
	play_mode_async			= 0x00000001,
	play_mode_loop			= 0x00000002,
};
typedef uint32_t PlayMode;


////////////////////////////////////////////////////////////////////////////////
void ECO_API play_sound(
	IN const char* filepath, 
	IN const PlayMode mode = play_mode_none);



////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif