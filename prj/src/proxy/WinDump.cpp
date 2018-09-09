#include "PrecHeader.h"
#include <eco/proxy/WinDump.h>
#include <eco/proxy/WinAutoHandler.h>
////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include <eco/proxy/Proxy.h>
#include <eco/date_time/DateTime.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#pragma comment(lib, "Dbghelp.lib")

////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(win);
static bool s_auto_restart = false;
////////////////////////////////////////////////////////////////////////////////
std::string get_dump_file_name()
{
	boost::system::error_code ec;
	// creat directory
	std::string file(".\\dump");
	boost::filesystem::path path(file);
	if (!boost::filesystem::exists(path, ec) &&
		!boost::filesystem::create_directory(path, ec))
	{
		EcoThrow(ec.value()) << ec.message();
	}

	// file name: \dump\eco.20180708121212.dump
	eco::date_time::Timestamp ts(eco::date_time::fmt_iso);
	file += "\\";
	file += eco::proxy::GetModuleName();
	file += '.';
	file += ts.get_value();
	file += ".dmp";
	return file;
}


////////////////////////////////////////////////////////////////////////////////
LONG WINAPI ExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	{
		// write dump.
		eco::win::AutoHandler file(::CreateFileA(get_dump_file_name().c_str(),
			GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
		if (file)
		{
			MINIDUMP_EXCEPTION_INFORMATION einfo;
			einfo.ThreadId = ::GetCurrentThreadId();
			einfo.ExceptionPointers = ExceptionInfo;
			einfo.ClientPointers = FALSE;
			::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(),
				file, MiniDumpWithFullMemory, &einfo, NULL, NULL);
		}
	}
	
	// restart app.
	if (s_auto_restart)
	{
		eco::proxy::RunExe(eco::proxy::GetModuleFile(), "", true);
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
void DisableSetUnhandledExceptionFilter()
{
	void *addr = (void*)GetProcAddress(
		LoadLibraryA("kernel32.dll"), "SetUnhandledExceptionFilter");
	if (addr)
	{
		unsigned char code[16];
		int size = 0;
		code[size++] = 0x33;
		code[size++] = 0xC0;
		code[size++] = 0xC2;
		code[size++] = 0x04;
		code[size++] = 0x00;
		DWORD dwOldFlag, dwTempFlag;
		HANDLE hdl = GetCurrentProcess();
		VirtualProtectEx(hdl, addr, size, PAGE_EXECUTE_READWRITE, &dwOldFlag);
		WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
		VirtualProtectEx(hdl, addr, size, dwOldFlag, &dwTempFlag);
	}
}


////////////////////////////////////////////////////////////////////////////////
void Dump::init(IN bool auto_restart)
{
	// currently this don't support win10.
	SYSTEM_INFO info;
	OSVERSIONINFOEX os;
	GetSystemInfo(&info);
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO *)&os) && os.dwMajorVersion <= 6)
	{
		s_auto_restart = auto_restart;
		::SetUnhandledExceptionFilter(&ExceptionFilter);
		DisableSetUnhandledExceptionFilter();
	}
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(win);
ECO_NS_END(eco);
#endif