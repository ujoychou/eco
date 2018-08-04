#ifndef ECO_NET_CODEC_H
#define ECO_NET_CODEC_H
/*******************************************************************************
@ name


@ function


@ exception


@ remark


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-12.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/net/Ecode.h>
#include <eco/Type.h>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class Codec
{
public:
	/*@ set message data to be encoded or to be decoded.*/
	virtual void* get_message() = 0;
	virtual void set_message(void* message) = 0;

	/*@ get bytes size of encoded message.*/
	virtual uint32_t get_byte_size() const = 0;

	/*@ append encode data string.*/
	virtual void encode_append(
		OUT eco::String& bytes,
		IN  const uint32_t size) const
	{
		uint32_t init_size = bytes.size();
		bytes.resize(init_size + size);
		encode(&bytes[init_size], size);
	}

	/*@ encode message to message bytes.
	* @ para.bytes: encoded message bytes.
	* @ para.size: bytes size of encoded message, return by "get_byte_size".
	*/
	virtual void encode(
		OUT char* bytes,
		IN  const uint32_t size) const = 0;

	// decode message from bytes string.
	virtual bool decode(
		IN const char* bytes,
		IN const uint32_t size)
	{
		return false;
	}	
};


////////////////////////////////////////////////////////////////////////////////
class TypeCodec : public eco::net::Codec
{
public:
	typedef void* (*MakeFunc)();

	template<typename type_t>
	inline static void* make()
	{
		return new type_t();
	}

public:
	inline explicit TypeCodec(IN Codec& codec, IN MakeFunc func)
		: m_make(func), m_codec(codec)
	{}

	virtual void set_message(void* message) override
	{
		m_codec.set_message(message);
	}

	virtual void* get_message() override
	{
		return m_codec.get_message();
	}

	virtual uint32_t get_byte_size() const override
	{
		return m_codec.get_byte_size();
	}

	virtual void encode(
		OUT char* bytes,
		IN  const uint32_t size) const override
	{
		m_codec.encode(bytes, size);
	}

	virtual bool decode(
		IN  const char* bytes,
		IN  const uint32_t size) override
	{
		m_codec.set_message(m_make());
		return m_codec.decode(bytes, size);
	}

private:
	Codec & m_codec;
	MakeFunc m_make;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif