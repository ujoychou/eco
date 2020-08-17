#ifndef ECO_CODEC_CODEC_H
#define ECO_CODEC_CODEC_H
/*******************************************************************************
@ name

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Export.h>
#include <eco/codec/md5.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(codec);
////////////////////////////////////////////////////////////////////////////////
inline std::string get_md5(IN const std::string& key)
{
	MD5 md(key);
	const char* digest = md.digest();
	const char* result = md.result();
	return std::string(result, 32);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(codec);
ECO_NS_END(eco);
#endif