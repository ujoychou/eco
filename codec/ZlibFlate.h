#ifndef ECO_CODEC_ZLIB_FLATE_H
#define ECO_CODEC_ZLIB_FLATE_H
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
#include <eco/codec/Zlib.h>


namespace eco{;
namespace codec{;
namespace zlib{;




////////////////////////////////////////////////////////////////////////////////
class Flate
{
////////////////////////////////////////////////////////////////////////////////
public:
	/*@ get protocol byte size.*/
	inline static uint32_t get_byte_size(IN const uint32_t msg_size)
	{
		return compressBound(msg_size);
	}

	/*@ compress "uncompress data" and assign it to a string value.
	* @ para.dest: return the compressed string data.
	* @ para.sour: uncompress data to be compressed.
	* @ para.sour_size: uncompress data size.
	*/
	template<typename String>
	inline static bool encode(
		OUT String& dest,
		IN  const char*  sour,
		IN  const uint32_t sour_size)
	{
		dest.resize(0);
		return append(dest, sour, sour_size);
	}

	/*@ same with "append_encode".*/
	template<typename String>
	inline static bool append(
		OUT String& dest,
		IN  const char*  sour,
		IN  const uint32_t sour_size)
	{
		return append_encode(dest, sour, sour_size);
	}

	/*@ compress "uncompress data" and append it to a string value.
	* @ para.dest: return the compressed string data.
	* @ para.sour: uncompress data to be compressed.
	* @ para.sour_size: uncompress data size.
	*/
	template<typename String>
	inline static bool append_encode(
		OUT String& dest,
		IN  const char*  sour,
		IN  const uint32_t sour_size)
	{
		if (sour_size == 0)
		{
			return false;
		}

		// forecast compress size space size.
		uLong result_size = compressBound(sour_size);
		if (result_size <= 0)
		{
			return false;
		}
		uint32_t init_size = dest.size();
		dest.resize(init_size + result_size);

		// compress process.
		int ret = compress2((Bytef*)&dest[init_size], &result_size, 
			(const Bytef*)(sour), sour_size, Z_BEST_COMPRESSION);
		if (ret != Z_OK)
		{
			dest.clear();
			return false;
		}

		// after "compress2" it has got precise compress size 
		// which is small than previous value, reset it to smaller.
		dest.resize(init_size + result_size);
		return true;
	}

	/*@ uncompress data when having known the uncompressed size.
	* @ para.dest: return the original string data.(uncompressed data)
	* @ para.data: compress data to be uncompressed.
	* @ para.sour_size: compress data size.
	* @ para.origin_size: the original data size before compress.
	*/
	template<typename String>
	inline static bool append_decode(
		OUT String& dest,
		IN  const char*  sour,
		IN  const uint32_t sour_size,
		IN  const uint32_t origin_size)
	{
		uLongf result_size = origin_size;
		uint32_t init_size = dest.size();
		dest.resize(init_size + result_size);
		int ret = uncompress((Bytef*)&dest[init_size], &result_size,
			(const Bytef*)sour, sour_size);
		if (result_size != origin_size)
		{
			assert(false);
			return false;
		}
		return (ret == Z_OK);
	}
};
////////////////////////////////////////////////////////////////////////////////

}// ns::zlib
}// ns::codec
}// ns::eco
////////////////////////////////////////////////////////////////////////////////
#endif