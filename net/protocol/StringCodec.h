#ifndef ECO_NET_STRING_CODEC_H
#define ECO_NET_STRING_CODEC_H
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
#include <eco/net/protocol/Codec.h>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class StringCodec : public eco::net::Codec
{
public:
	explicit inline StringCodec(IN const uint32_t capacity = 0)
	{
		m_msg.reserve(capacity);
	}

	explicit inline StringCodec(IN const char* msg)
	{
		m_msg.asign(msg);
	}

	inline StringCodec(IN eco::String&& msg) : m_msg((eco::String&&)msg)
	{}

	inline void reserve(IN const uint32_t capacity)
	{
		m_msg.reserve(capacity);
	}

	inline void append(IN const char* msg, IN const uint32_t siz)
	{
		m_msg.append(msg, siz);
	}

public:
	// implement the Codec interface.
	virtual void set_message(void* message)
	{
		m_msg.asign(static_cast<const char*>(message));
	}

	virtual uint32_t get_byte_size() const
	{
		return m_msg.size();
	}

	virtual void encode(
		OUT char* bytes,
		IN  const uint32_t size) const
	{
		uint32_t siz = size > m_msg.size() ? m_msg.size() : size;
		memcpy(bytes, m_msg.c_str(), siz);
	}

	virtual bool decode(
		IN  const char* bytes,
		IN  const uint32_t size) override
	{
		m_msg.asign(bytes, size);
		return true;
	}

private:
	eco::String m_msg;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif