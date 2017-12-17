#ifndef ECO_OS_PROXY_H
#define ECO_OS_PROXY_H
////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
@ name
os proxy, as a adapter for all operation system.

@ function
as


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-01-15.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Project.h>


namespace eco{;
namespace proxy{;
////////////////////////////////////////////////////////////////////////////////

// Open the file in the operation system.
ECO_API bool OpenFile(
	IN const std::string& szFILE_PATH);

// Execute the exe.
ECO_API bool RunExe(
	IN const std::string& szEXE_FILE,
	IN const std::string& szPARAS,
	IN bool bShow = false);

// Execute the exe.
ECO_API bool RunOnceExe(
	IN const std::string& szEXE_FILE,
	IN const std::string& szPARAS,
	IN bool bShow = false);

// Verify the exe is running.
ECO_API bool IsExeFileRunning(
	IN const std::string& exe_path);

// Terminate the process.
ECO_API bool TerminateExeFile(
	IN const std::string& exe_path);

// Verify the exe is running.
ECO_API bool IsExeRunning(
	IN const std::string& exe_name);

// Terminate the process.
ECO_API bool TerminateExe(
	IN const std::string& exe_name);

// Get current module directory
ECO_API void GetRelativeDirectory(
	OUT std::string& dest_dir,
	IN  const std::string& relative_dir);

// Get current module directory
ECO_API void GetModuleDirectory(
	OUT std::string& dest_dir);

// Get system date: 2014-07-07
ECO_API std::string GetSystemDate();

// Get system time: 12:50:55
ECO_API std::string GetSystemTime();

// Get system datetime: 2014-07-07 12:50:55.
ECO_API std::string GetSystemDateTime();


////////////////////////////////////////////////////////////////////////////////
}}
////////////////////////////////////////////////////////////////////////////////
#endif