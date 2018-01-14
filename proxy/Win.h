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
inline void remove_end(OUT std::string& tmp)
{
	size_t x = 0;
	size_t i = tmp.length() - 1;
	for (; i > 0; --i)
	{
		if (tmp[i] != 0)
			break;
		++x;
	}
	if (i == 0 && tmp[i] == 0) ++x;
	tmp.resize(tmp.length() - x);
}

inline void utf_to_gbk(OUT std::string& gbk, IN const char* utf8)
{
	std::wstring wstr;
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wstr.resize(len);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &wstr[0], len);

	len = WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, NULL, 0, NULL, NULL);
	gbk.resize(len);
	WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, &gbk[0], len, NULL, NULL);
	remove_end(gbk);
}


////////////////////////////////////////////////////////////////////////////////
inline void gbk_to_utf(OUT std::string& utf, IN const char* gbk)
{
	std::wstring wstr;
	int len = MultiByteToWideChar(CP_ACP, 0, gbk, -1, NULL, 0);
	wstr.resize(len);
	MultiByteToWideChar(CP_ACP, 0, gbk, -1, &wstr[0], len);

	len = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, NULL, 0, NULL, NULL);
	utf.resize(len);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, &utf[0], len, NULL, NULL);
	remove_end(utf);
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif