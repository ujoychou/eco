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
#include <string>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class Crypt
{
public:
	/*@ message bytes size after encode.*/
	virtual uint32_t get_byte_size(IN uint32_t size)
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
		IN const uint32_t encode_size,
		IN eco::Error& e)
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
		IN const uint32_t start_pos,
		IN eco::Error& e)
	{
		return decode(encode_str, start_pos, encode_str.size() - start_pos, e);
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
		uint16_t origin_size = 
			static_cast<uint16_t>(origin_str.size() - start_pos);
		uint16_t encode_size = Coder::get_byte_size(origin_size);

		// append "exist data".
		eco::String encode_str;
		encode_str.reserve(start_pos + sizeof(uint16_t) + encode_size);
		encode_str.append(origin_str.c_str(), start_pos);
		// append "original data length".
		append_network_int(encode_str, origin_size);
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
		IN const uint32_t encode_size,
		IN eco::Error& e) override
	{
		if (encode_size <= sizeof(uint16_t))	// original data length
		{
			e.id(e_message_decode) << "message has no 'original data length'";
			return eco::String();
		}

		// decode: original data size.
		uint16_t origin_size = network_to_host_int16(&encode_str[start_pos]);
		uint16_t encode_data_size = encode_size - sizeof(uint16_t);
		if (origin_size < 1 || encode_data_size < 1)
		{
			e.id(e_message_decode) << "message original data length "
				"or encoded data length = 0.";
			return eco::String();
		}

		// decode: original data.
		eco::String origin_str;
		origin_str.reserve(start_pos + origin_size);
		origin_str.append(&encode_str[0], start_pos);
		const char* encode_str = &encode_str[start_pos] + sizeof(uint16_t);
		if (!Coder::append_decode(origin_str, 
			encode_str, encode_data_size, origin_size))
		{
			e.id(e_message_decode) << "coder append decode error.";
			return eco::String();
		}
		return std::move(origin_str);
	}
};


}}
////////////////////////////////////////////////////////////////////////////////
#endif