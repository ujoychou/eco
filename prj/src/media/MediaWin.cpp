#include "PrecHeader.h"
#ifdef ECO_WIN32
#include <eco/media/Media.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include "windows.h"
#pragma comment(lib, "winmm.lib")


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
// windows dll load implement.
void play_sound(IN const char* filepath, IN const PlayMode mode)
{
	DWORD flag = SND_FILENAME | SND_NOSTOP;
	if (eco::has(mode, play_mode_async))
		flag |= SND_ASYNC;
	if (eco::has(mode, play_mode_loop))
		flag |= SND_LOOP;
	::PlaySoundA(filepath, NULL, flag);
}


////////////////////////////////////////////////////////////////////////////////
}
#endif