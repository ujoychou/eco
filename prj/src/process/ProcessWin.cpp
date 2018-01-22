#include "PrecHeader.h"
#ifdef ECO_WIN
#include <eco/process/Process.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Cast.h>
#include "Process.h"



////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace this_process{;
static char s_pid_str[64] = {0};
size_t get_id()
{
	return ::getpid();
}
const char* get_id_string()
{
	if (s_pid_str[0] == '\0')
	{
		eco::Integer<int> caster(::getpid(), eco::dec, 8);
		strcpy(s_pid_str, caster.c_str());
	}
	return s_pid_str;
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif