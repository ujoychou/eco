#include "Pch.h"
#ifdef ECO_WIN
#include <eco/proc/Process.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Cast.h>
#include <process.h>    // windows header


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(this_process);
////////////////////////////////////////////////////////////////////////////////
static char s_pid[16] = {0};
size_t id() { return (size_t)::getpid(); }

const char* sid()
{
    if (eco::empty(s_pid))
    {
        eco_cpyc(s_pid, eco::Integer<size_t>(id(), eco::dec, 8).c_str());
    }
}
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(this_process);
ECO_NS_END(eco);
#endif

