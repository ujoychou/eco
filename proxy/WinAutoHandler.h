#ifndef ECO_WIN_AUTO_HANDLER_H
#define ECO_WIN_AUTO_HANDLER_H
////////////////////////////////////////////////////////////////////////////////
#include "windows.h"


namespace eco{;
namespace win{;
////////////////////////////////////////////////////////////////////////////////
class AutoHandler
{
public:
	inline AutoHandler(IN HANDLE hdl) : m_handle(hdl)
	{}

	inline ~AutoHandler()
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
		return (m_handle != nullptr && m_handle != INVALID_HANDLE_VALUE);
	}

private:
	HANDLE m_handle;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif