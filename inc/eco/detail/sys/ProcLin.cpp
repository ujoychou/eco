#include "Pch.h"
#ifdef ECO_LINUX
#include <eco/proc/Proc.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Cast.h>
#include <eco/String.h>
#include <unistd.h>     // linux header


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
    return s_pid;
}
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(this_process);
ECO_NS_END(eco);
#endif

