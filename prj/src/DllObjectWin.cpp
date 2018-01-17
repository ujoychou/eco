#include "PrecHeader.h"
#ifdef ECO_WIN32
#include <eco/DllObject.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include "windows.h"


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
// windows dll load implement.
void* load_dll(IN const char* dll_name)
{
	void* handler = static_cast<void*>(::LoadLibraryA(dll_name));
	if (handler == nullptr)
	{
		uint32_t eid = ::GetLastError();
		EcoThrow(eid) << "load windows library fail: " << dll_name;
	}
	return handler;
}


////////////////////////////////////////////////////////////////////////////////
void  free_dll(IN void*& dll_handle)
{
	::FreeLibrary(static_cast<HMODULE>(dll_handle));
	dll_handle = nullptr;
}


////////////////////////////////////////////////////////////////////////////////
DllFunc get_function(IN void* dll_handle, IN const char* func_name)
{
	DllFunc func = reinterpret_cast<DllFunc>(
		::GetProcAddress(static_cast<HMODULE>(dll_handle), func_name));
	if (func == nullptr)
	{
		uint32_t eid = ::GetLastError();
		EcoThrow(eid) << "load windows library fail: " << func_name;
	}
	return func;
}


////////////////////////////////////////////////////////////////////////////////
}
#endif