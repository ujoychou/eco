#ifndef ECO_WIN_H
#define ECO_WIN_H
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
#include <string>
#include <eco/Memory.h>
#include "windows.h"


namespace eco{;
namespace win{;


////////////////////////////////////////////////////////////////////////////////
inline void cast(OUT std::string& dest, IN  const wchar_t* sour)
{
	int wlen = static_cast<int>(wcslen(sour));
	int alen = WideCharToMultiByte(CP_ACP, 0, sour, wlen, NULL, 0, NULL, NULL);
	dest.resize(alen);
	WideCharToMultiByte(CP_ACP, 0, sour, wlen, &dest[0], alen, NULL, NULL);
}
inline std::string cast(IN  const wchar_t* sour)
{
	std::string msg;
	cast(msg, sour);
	return msg;
}


////////////////////////////////////////////////////////////////////////////////
inline void cast(OUT std::wstring& dest, IN  const char* sour)
{
	int alen = static_cast<int>(strlen(sour));
	int wlen = MultiByteToWideChar(CP_ACP, 0, sour, alen, NULL, 0);
	dest.resize(wlen);
	MultiByteToWideChar(CP_ACP, 0, sour, alen, &dest[0], wlen);
}
inline std::wstring cast(IN  const char* sour)
{
	std::wstring msg;
	cast(msg, sour);
	return msg;
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif