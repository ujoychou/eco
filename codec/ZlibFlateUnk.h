#ifndef ECO_CODEC_ZLIB_FLATE_UNK_H
#define ECO_CODEC_ZLIB_FLATE_UNK_H
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
#include <eco/codec/ZlibFlate.h>


namespace eco{;
namespace codec{;
namespace zlib{;

////////////////////////////////////////////////////////////////////////////////
class FlateUnk : public Flate
{
public:
	/*@ uncompress data when unknown the uncompressed size.
	* @ para.result: return the original string data.(uncompressed data)
	* @ para.sour: compress data to be uncompressed.
	* @ para.sour_size: compress data size.
	* @ para.origin_size: no use this parameter.
	*/
	inline static bool decode(
		OUT std::string& result,
		IN  const char*  sour,
		IN  const size_t sour_size)
	{
		if (sour_size == 0)
		{
			return false;
		}

		// inflate stream wrapper object.
		StreamObject obj(sour, sour_size);
		if (!obj.InflateInit())
		{
			return false;
		}

		// uncompress data unit by unit.
		int ret = Z_OK;
		size_t init_size = result.size();
		size_t last_pos = init_size;
		const size_t uncomp_unit_size = sour_size;
		do{
			// alloc output memory.
			result.append(uncomp_unit_size, 0);
			ret = obj.Inflate(&result[last_pos], uncomp_unit_size);
			if (ret < Z_OK)
			{
				return false;
			}
			last_pos += uncomp_unit_size; // reset new output memory pos.
		}while (obj.HasMoreData(ret));

		// if lastest time unit uncompress not fullfill the memory, reset it.
		result.resize(init_size + obj.m_strm.total_out);
		return obj.InflateEnd();
	}

////////////////////////////////////////////////////////////////////////////////
private:
	/*@ z_stream wrapper for auto manage it's resource.*/
	class StreamObject
	{
	public:
		inline StreamObject(
			IN const void* comp_data,
			IN const uInt comp_size)
		{
			std::memset(&m_strm, 0, sizeof(m_strm));
			m_strm.zalloc = (alloc_func)0;
			m_strm.zfree = (free_func)0;

			// set input compress data.
			
			m_strm.next_in = const_cast<Bytef*>(
				static_cast<const Bytef*>(comp_data));
			m_strm.avail_in = comp_size;
		}

		inline ~StreamObject()
		{
			InflateEnd();
		}

		inline bool InflateInit()
		{
			int ret = inflateInit(&m_strm);
			return (ret == Z_OK);
		}
		inline bool InflateEnd()
		{
			int ret = inflateEnd(&m_strm);
			return (ret != Z_OK);
		}

		inline int Inflate(
			IN void* uncomp_unit,
			IN const uInt uncomp_unit_size)
		{
			m_strm.next_out = static_cast<Bytef*>(uncomp_unit);
			m_strm.avail_out = uncomp_unit_size;
			return inflate(&m_strm, Z_NO_FLUSH);
		}

		inline bool HasMoreData(IN const int ret)
		{
			// note that avail_out must be 0.
			return (m_strm.avail_out == 0 && ret != Z_STREAM_END);
		}

	public:
		z_stream m_strm;
	};
};


}// ns::zlib
}// ns::codec
}// ns::eco
////////////////////////////////////////////////////////////////////////////////
#endif