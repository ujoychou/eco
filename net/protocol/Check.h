#ifndef ECO_NET_CHECK_H
#define ECO_NET_CHECK_H
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
#include <eco/net/Net.h>
#include <exception>



namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class Check
{
public:
	virtual uint32_t get_byte_size()
	{
		return 0;
	}

	/*@ compute bytes checksum and append it as a string to bytes.
	* @ para.bytes: bytes stream being compute checksum.
	* @ para.checksum_start: from where bytes stream being compute checksum.
	*/
	virtual void encode_append(
		OUT eco::String& bytes,
		IN  const uint32_t checksum_start) = 0;

	/*@ verify bytes stream checksum is correct.
	* @ para.bytes: bytes stream being verified checksum.
	* @ para.size: bytes stream size.
	* @ para.start_pos: string checksum start pos.
	*/
	virtual bool decode(
		IN const char* bytes,
		IN const size_t size,
		IN const uint32_t start) = 0;

public:
	/*@ same with "encode".*/
	inline void encode(OUT eco::String& bytes)
	{
		encode(bytes, 0);
	}

	/*@ same with "append_encode".*/
	inline void encode(
		OUT eco::String& bytes,
		IN  const uint32_t checksum_start)
	{
		encode_append(bytes, checksum_start);
	}

	/*@ same with "decode".*/
	inline bool decode(OUT eco::String& bytes)
	{
		return decode(bytes.c_str(), bytes.size(), 0);
	}
};




////////////////////////////////////////////////////////////////////////////////
/*@ typedef bytes checksum function type, which return a uint32_t value.
* @ para.bytes: bytes stream.
* @ para.size: bytes stream size.
*/
typedef uint32_t (*CheckSum32Func)(IN const char* bytes, IN uint32_t size);


////////////////////////////////////////////////////////////////////////////////
template<CheckSum32Func func, typename CheckSumType = uint32_t>
class CheckSumProtocol : public Check
{
public:
	/*@ get sync protocol byte size.*/
	virtual uint32_t get_byte_size() override
	{
		return sizeof(CheckSumType);
	}

	/*@ compute bytes checksum and append it as a string to bytes.
	* @ para.bytes: bytes stream being compute checksum.
	* @ para.checksum_start: from where bytes stream being compute checksum.
	*/
	virtual void encode_append(
		OUT eco::String& bytes,
		IN  const uint32_t checksum_start) override
	{
		using namespace boost::asio::detail::socket_ops;
		assert(bytes.size() > checksum_start);
		const char* start = bytes.c_str() + checksum_start;
		uint32_t size = bytes.size() - checksum_start;
		uint32_t checksum = func(start, size);
		append_network_int(bytes, checksum);
	}

	/*@ verify bytes stream checksum is correct.
	* @ para.bytes: bytes stream being verified checksum.
	* @ para.size: bytes stream size.
	* @ para.start: string checksum start pos.
	*/
	virtual bool decode(
		IN const char* bytes,
		IN const size_t size,
		IN const uint32_t start) override
	{
		int32_t checksum_bytes_size = size - start - get_byte_size();
		if (checksum_bytes_size < 1)
		{
			return false;
		}
		
		uint32_t origin_checksum = network_to_host_int32(
			&bytes[start + checksum_bytes_size]);
		uint32_t checksum = func(&bytes[start], checksum_bytes_size);
		if (checksum != origin_checksum)
		{
			return false;
		}
		return true;
	}// end decode.
};


}// ns::net
}// ns::eco
////////////////////////////////////////////////////////////////////////////////
#endif