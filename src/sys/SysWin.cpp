#include "Pch.h"
#include <eco/sys/Sys.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/sys/WinAutoHandler.h>
#include <eco/filesystem/Path.h>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string.hpp>

#ifdef ECO_WIN
// windows header and lib.
#include "windows.h"
#include "shellapi.h"
#include "tlhelp32.h"
#include "psapi.h"
#pragma comment(lib, "psapi.lib")


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(sys);
////////////////////////////////////////////////////////////////////////////////
inline bool GetProcessFilePath(
	OUT std::string& proc_path,
	IN  const DWORD pid)
{
	win::AutoHandler proc(OpenProcess(
		PROCESS_QUERY_INFORMATION |PROCESS_VM_READ, FALSE, pid));
	if (!proc)
	{
		return false;
	}

	DWORD need = 0;
	HMODULE proc_mod = NULL;
	if(!EnumProcessModules(proc, &proc_mod, sizeof(proc_mod), &need))
	{
		return false;
	}

	TCHAR full_path[MAX_PATH] = {0};
	if (GetModuleFileNameEx(proc, proc_mod, full_path, MAX_PATH) == 0)
	{
		return false;
	}
	boost::filesystem::path fmt(full_path);
	proc_path = fmt.string();
	return true;
}


////////////////////////////////////////////////////////////////////////////////
inline int FindWin32ProcessId(
	OUT DWORD& pid,
	IN const std::string& exe_name)
{
	// create process snapshot.
	win::AutoHandler snapshot(::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
	if (!snapshot)
	{
		return eco::error;
	}

	// traverse process snapshot.
	PROCESSENTRY32 entry = {0};
	entry.dwSize = sizeof(PROCESSENTRY32);
	BOOL is_find = Process32First(snapshot, &entry);
	while (is_find)
	{
		// find exe process.
		if(boost::iequals(exe_name,
			boost::filesystem::path(entry.szExeFile).string()))
		{
			pid = entry.th32ProcessID;
			return eco::ok;
		}
		is_find = Process32Next(snapshot, &entry);
	}
	return eco::fail;
}


////////////////////////////////////////////////////////////////////////////////
inline int FindWin32ProcessIdByPath(
	OUT DWORD& pid,
	IN  const std::string& exe_path)
{
	boost::filesystem::path exe_name = 
		boost::filesystem::path(exe_path).filename();

	// create process snapshot.
	win::AutoHandler snapshot(::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
	if (!snapshot)
	{
		return eco::error;
	}

	// traverse process snapshot.
	PROCESSENTRY32 entry = {0};
	entry.dwSize = sizeof(PROCESSENTRY32);
	for (BOOL is_find = Process32First(snapshot, &entry); is_find;
		is_find = Process32Next(snapshot, &entry))
	{
		// find process by name.
		boost::filesystem::path cur_exe_name(entry.szExeFile);
		if(!boost::iequals(exe_name.string(), cur_exe_name.string()))
		{
			continue;
		}

		// find process by exe path.
		std::string cur_exe_path;
		if (!GetProcessFilePath(cur_exe_path, entry.th32ProcessID))
		{
			return eco::error;
		}
		if (eco::filesystem::equal(exe_path, cur_exe_path))
		{
			pid = entry.th32ProcessID;
			return eco::ok;
		}
	}
	return eco::fail;
}


////////////////////////////////////////////////////////////////////////////////
bool OpenFile(IN const std::string& szFILE_PATH)
{
	SHELLEXECUTEINFOA iShell = {0};
	iShell.cbSize = sizeof(iShell);
	iShell.fMask = SEE_MASK_NOCLOSEPROCESS;
	iShell.lpFile = szFILE_PATH.c_str();
	iShell.lpVerb = ("open");
	return ShellExecuteExA(&iShell) == TRUE;
}


////////////////////////////////////////////////////////////////////////////////
bool RunExe(
	IN const std::string& szEXE_FILE,
	IN const std::string& szPARAS,
	IN bool bShow,
	IN bool bSync)
{
	SHELLEXECUTEINFOA iShell = {0};
	iShell.cbSize = sizeof(iShell);
	iShell.fMask = SEE_MASK_NOCLOSEPROCESS;
	iShell.lpFile = szEXE_FILE.c_str();
	if (!szPARAS.empty())
	{
		iShell.lpParameters = szPARAS.c_str();
	}
	iShell.lpVerb = ("open");
	iShell.nShow = bShow ? SW_SHOW : SW_HIDE;
	if (ShellExecuteExA(&iShell) != TRUE)
	{
		return false;
	}

	// sync wait for process end.
	if (bSync)
	{
		WaitForSingleObject(iShell.hProcess, INFINITE);
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////
bool TerminateExeFile(IN const std::string& exe_path)
{
	DWORD pid = 0;
	while (true)
	{
		int res = FindWin32ProcessIdByPath(pid, exe_path);
		if (res != eco::ok)
		{
			return false;
		}
		win::AutoHandler proc(::OpenProcess(PROCESS_TERMINATE, FALSE, pid));
		if (!proc)
		{
			return false;
		}
		::TerminateProcess(proc, 0);
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////
bool IsExeFileRunning(IN const std::string& exe_path)
{
	DWORD pid = 0;
	return FindWin32ProcessIdByPath(pid, exe_path) == eco::ok;
}


////////////////////////////////////////////////////////////////////////////////
bool TerminateExe(IN const std::string& exe_name)
{
	DWORD pid = 0;
	while (true)
	{
		int res = FindWin32ProcessId(pid, exe_name);
		if (res != eco::ok)
		{
			return false;
		}
		win::AutoHandler proc(::OpenProcess(PROCESS_TERMINATE, FALSE, pid));
		if (!proc)
		{
			return false;
		}
		::TerminateProcess(proc, 0);
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////
bool IsExeRunning(IN const std::string& exe_name)
{
	DWORD pid = 0;
	return FindWin32ProcessId(pid, exe_name) == eco::ok;
}


////////////////////////////////////////////////////////////////////////////////
std::string GetRelativePath(IN  const std::string& relative_dir)
{
	std::string dest_dir = GetAppPath();
	dest_dir += '/';
	return dest_dir += relative_dir;
}


////////////////////////////////////////////////////////////////////////////////
std::string GetAppFile()
{
	char module_file[2048];
	GetModuleFileNameA(NULL, module_file, 2048);
	return module_file;
}
std::string GetAppPath()
{
	char module_file[2048];
	GetModuleFileNameA(NULL, module_file, 2048);
	// Get the parent directory.
	return boost::filesystem::path(module_file).parent_path().string();
}
std::string GetAppName()
{
	char path[2048] = { 0 };
	GetModuleFileNameA(NULL, path, 2048);
	uint32_t start = eco::find_last(path, '\\');
	if (start == std::string::npos) return eco::value_empty;
	uint32_t end = eco::find_last(path, '.');
	if (end == std::string::npos) return eco::value_empty;
	std::string result(&path[start + 1], end - start - 1);
	return result;
}


////////////////////////////////////////////////////////////////////////////////
std::string GetModuleFile(void* func_addr)
{
	// get current module dll handle.
	HMODULE hdl = NULL;
	if (func_addr != nullptr)
	{
		MEMORY_BASIC_INFORMATION mbi;
		if (::VirtualQuery(func_addr, &mbi, sizeof(mbi)) != 0)
			hdl = (HMODULE)mbi.AllocationBase;
	}

	// get dll file path by handle.
	char file[2048];
	GetModuleFileNameA(hdl, file, 2048);
	return file;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(sys);
ECO_NS_END(eco);
#endif