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
	inline StringCodec(void* msg = nullptr) : eco::net::Codec(msg)
	{}

	const std::string& message() const
	{
		return *(std::string*)m_msg;
	}

	virtual void set_message(void* message) override
	{
		m_msg = static_cast<std::string*>(message);
	}

	virtual uint32_t byte_size() const override
	{
		return (uint32_t)message().size();
	}

	virtual void encode(OUT char* bytes, IN uint32_t size) const override
	{
		memcpy(bytes, message().c_str(), size);
	}

	virtual void* decode(IN const char* bytes, IN uint32_t size) override
	{
		auto& msg = *(std::string*)m_msg;
		msg.assign(bytes, size);
		return m_msg;
	}
};

////////////////////////////////////////////////////////////////////////////////
}}
#endif