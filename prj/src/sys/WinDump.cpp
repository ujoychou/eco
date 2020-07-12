#include "PrecHeader.h"
#include <eco/sys/WinDump.h>
#include <eco/sys/WinAutoHandler.h>
////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include <eco/thread/Mutex.h>
#include <eco/sys/Sys.h>
#include <eco/date_time/DateTime.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#pragma comment(lib, "Dbghelp.lib")

////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(win);
static bool s_auto_restart = false;
static bool s_init = false;
static eco::Mutex s_mutex;
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
		ECO_THROW(ec.value()) << ec.message();
	}

	// file name: \dump\eco.20180708121212.dump
	eco::date_time::Timestamp ts(eco::date_time::fmt_iso);
	file += "\\";
	file += eco::sys::GetAppName();
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
		eco::sys::RunExe(eco::sys::GetAppFile(), "", true);
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
	eco::Mutex::ScopeLock lock(s_mutex);
	if (s_init) return;

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
	s_init = true;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(win);
ECO_NS_END(eco);
#endif