#ifndef ECO_CODEC_ZLIB_H
#define ECO_CODEC_ZLIB_H
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
#include <eco/Project.h>

// zlib ref
#ifndef ZLIB_WINAPI
#	define ZLIB_WINAPI
#endif
#include <zlib/zlib.h>


namespace eco{;
namespace codec{;
namespace zlib{;

////////////////////////////////////////////////////////////////////////////////
/*@ generate data bytes "check sum". 
* @ para.data: data bytes.
* @ para.size: data size.
*/
inline uint32_t adler32(IN const char* data, IN uint32_t size)
{
	return static_cast<uint32_t>(
		::adler32(1, reinterpret_cast<const Bytef*>(data), size));
}


////////////////////////////////////////////////////////////////////////////////
}// ns::zlib
}// ns::codec
}// ns::eco
////////////////////////////////////////////////////////////////////////////////
#endif