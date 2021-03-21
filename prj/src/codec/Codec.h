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


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(codec);
////////////////////////////////////////////////////////////////////////////////
std::string get_md5(IN const std::string& key);


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(codec);
ECO_NS_END(eco);
#endif