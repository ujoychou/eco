#ifndef ECO_NET_CRYPT_H
#define ECO_NET_CRYPT_H
/*******************************************************************************
@ name
ByteCodeProtocol

@ function
protocol stack structure like this:
1.head: bytes length: uint16_t
2.original message length: uint16_t
3.encode message.

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/Type.h>
#include <eco/codec/ZlibFlate.h>
#include <string>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class Crypt
{
public:
	/*@ message bytes size after encode.*/
	virtual uint32_t byte_size(IN uint32_t size)
	{
		return size;
	}

	/*@ replace with encoded string.
	* @ return: encoded string.
	* @ para.origin_str: input original string data.
	* @ para.start_pos: where encoded string start.
	*/
	virtual eco::String append_encode(
		IN  eco::String& origin_str,
		IN  uint32_t start_pos)
	{
		return std::move(origin_str);
	}

	/*@ decode string.
	* @ return: return decoded original string.
	* @ para.encode_str: input encoded string.
	* @ para.start_pos: where encoded string start.
	* @ para.encode_size: encoded string size.
	*/
	virtual eco::String decode(
		IN eco::String& encode_str,
		IN const uint32_t start_pos,
		IN const uint32_t encode_size)
	{
		return std::move(encode_str);
	}

public:
	/*@ encode string.
	* @ return: encoded string.
	* @ para.origin_str: input original string data.
	*/
	inline eco::String encode(
		IN eco::String& origin_str)
	{
		return encode(origin_str, 0);
	}

	// same with "append_encode".
	inline eco::String encode(
		IN  eco::String& origin_str,
		IN  uint32_t start_pos)
	{
		return append_encode(origin_str, start_pos);
	}

	/*@ decode string.
	* @ return: decoded original string.
	* @ para.encode_str: input encoded string.
	* @ para.start_pos: where encoded string start.
	*/
	inline eco::String decode(
		IN eco::String& encode_str,
		IN const uint32_t start_pos)
	{
		return decode(encode_str, start_pos, encode_str.size() - start_pos);
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename Coder>
class CryptT : public Crypt
{
public:
	/*@ message bytes size after encode.*/
	virtual uint32_t get_byte_size(IN uint32_t size)
	{
		return size + ((size < 80) ? 20 : 0);
	}

	/*@ replace with encoded string.
	* @ return: encoded string.
	* @ para.origin_str: input original string data.
	* @ para.start_pos: where encoded string start.
	*/
	virtual eco::String append_encode(
		IN  eco::String& origin_str,
		IN  uint32_t start_pos)
	{
		uint32_t origin_size = 
			static_cast<uint32_t>(origin_str.size() - start_pos);
		uint16_t encode_size = Coder::get_byte_size(origin_size);

		// append "exist data".
		eco::String encode_str;
		encode_str.reserve(start_pos + sizeof(uint32_t) + encode_size);
		encode_str.append(origin_str.c_str(), start_pos);
		// append "original data length".
		append_hton(encode_str, origin_size);
		// append "encoded data".
		if (!Coder::append_encode(
			encode_str, &origin_str[start_pos], origin_size))
		{
			return eco::String();
		}
		return std::move(encode_str);
	}

	/*@ decode string.
	* @ return: return decoded original string.
	* @ para.encode_str: input encoded string.
	* @ para.start_pos: where encoded string start.
	* @ para.encode_size: encoded string size.
	*/
	virtual eco::String decode(
		IN eco::String& encode_str,
		IN const uint32_t start_pos,
		IN const uint32_t encode_size) override
	{
		const int sol = sizeof(uint32_t);	// size of original data length.
		if (encode_size <= sol)				// original data length
		{
			ECO_THIS_ERROR(e_message_decode)
				< "message has no 'original data length'";
			return eco::String();
		}

		// decode: original data size.
		uint32_t origin_size = ntoh32(&encode_str[start_pos]);
		uint32_t encode_data_size = encode_size - sol;
		if (origin_size < 1 || encode_data_size < 1)
		{
			ECO_THIS_ERROR(e_message_decode)
				< "message original data length or encoded data length = 0.";
			return eco::String();
		}

		// decode: original data.
		eco::String origin_str;
		origin_str.reserve(start_pos + origin_size);
		origin_str.append(&encode_str[0], start_pos);
		if (!Coder::append_decode(origin_str,
			&encode_str[start_pos + sol], encode_data_size, origin_size))
		{
			ECO_THIS_ERROR(e_message_decode) < "coder append decode error.";
			return eco::String();
		}
		return std::move(origin_str);
	}
};
typedef eco::net::CryptT<eco::codec::zlib::Flate> CryptFlate;


}}
////////////////////////////////////////////////////////////////////////////////
#endif