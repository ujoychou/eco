#include "Pch.h"
#ifdef ECO_WIN32
#include <eco/rx/RxDll.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Error.h>
#include "windows.h"


ECO_NS_BEGIN(eco);
// windows dll load implement.
////////////////////////////////////////////////////////////////////////////////
void* dll_load(IN const char* dll_name)
{
	void* handler = static_cast<void*>(::LoadLibraryA(dll_name));
	if (handler == nullptr)
	{
		uint32_t eid = ::GetLastError();
		ECO_THROW("load windows library fail: ") << dll_name;
	}
	return handler;
}


////////////////////////////////////////////////////////////////////////////////
void  dll_free(IN void*& dll_handle)
{
	::FreeLibrary(static_cast<HMODULE>(dll_handle));
	dll_handle = nullptr;
}


////////////////////////////////////////////////////////////////////////////////
RxFunc dll_func(IN void* dll_handle, IN const char* func_name)
{
	RxFunc func = reinterpret_cast<RxFunc>(
		::GetProcAddress(static_cast<HMODULE>(dll_handle), func_name));
	if (func == nullptr)
	{
		uint32_t eid = ::GetLastError();
		ECO_THROW("load windows library fail: ") < func_name;
	}
	return func;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif