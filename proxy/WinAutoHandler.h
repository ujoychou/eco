#ifndef ECO_PROXY_AUTO_HANDLER_H
#define ECO_PROXY_AUTO_HANDLER_H
////////////////////////////////////////////////////////////////////////////////
#include "windows.h"


namespace eco{;
namespace proxy{;
////////////////////////////////////////////////////////////////////////////////
class WinAutoHandler
{
public:
	inline WinAutoHandler(IN HANDLE hdl) : m_handle(hdl)
	{}

	inline ~WinAutoHandler()
	{
		if (m_handle != nullptr)
		{
			::CloseHandle(m_handle);
		}
	}

	inline operator HANDLE()
	{
		return m_handle;
	}

	inline operator const HANDLE() const
	{
		return m_handle;
	}

	inline operator bool() const
	{
		return (m_handle != nullptr);
	}

private:
	HANDLE m_handle;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif