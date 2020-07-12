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
#include <eco/Type.h>
#include "windows.h"


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(win);
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
inline void utf8_to_gbk(OUT std::string& gbk, IN const char* utf8)
{
	// utf8 to unicode
	std::wstring wstr;
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wstr.resize(len);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &wstr[0], len);
	// unicode to gbk.
	len = WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, NULL, 0, NULL, NULL);
	gbk.resize(len);
	WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, &gbk[0], len, NULL, NULL);
	eco::fit(gbk);
}
inline void utf8_to_gbk(OUT std::string& val)
{
	utf8_to_gbk(val, val.c_str());
}


////////////////////////////////////////////////////////////////////////////////
inline void gbk_to_utf8(OUT std::string& utf, IN const char* gbk)
{
	// gbk to unicode
	std::wstring wstr;
	int len = MultiByteToWideChar(CP_ACP, 0, gbk, -1, NULL, 0);
	wstr.resize(len);
	MultiByteToWideChar(CP_ACP, 0, gbk, -1, &wstr[0], len);
	// unicode to utf8.
	len = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, NULL, 0, NULL, NULL);
	utf.resize(len);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], -1, &utf[0], len, NULL, NULL);
	eco::fit(utf);
}
inline void gbk_to_utf8(OUT std::string& val)
{
	gbk_to_utf8(val, val.c_str());
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(win);
ECO_NS_END(eco);
#endif