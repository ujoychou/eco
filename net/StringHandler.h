#ifndef ECO_NET_STRING_HANDLER_H
#define ECO_NET_STRING_HANDLER_H
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-17.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/net/MessageHandler.h>
#include <eco/net/protocol/codec.h>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class StringCodec : public eco::net::Codec
{
public:
	explicit StringCodec(
		IN const std::string& msg)
		: m_msg(msg)
	{}

	virtual uint32_t get_byte_size() const override
	{
		return m_msg.size();
	}

	virtual void encode(
		OUT eco::String& bytes,
		IN  const uint32_t size) const override
	{
		bytes.append(m_msg.c_str(), m_msg.size());
	}

	static inline bool decode(
		OUT std::string& msg,
		IN  const char* bytes,
		IN  const uint32_t size)
	{
		msg.assign(bytes, size);
		return true;
	}

private:
	const std::string& m_msg;
};


////////////////////////////////////////////////////////////////////////////////
class StringHandler : public MessageHandlerT<std::string>
{
public:
	virtual bool decode(
		IN const char* bytes,
		IN const uint32_t size) override
	{
		return StringCodec::decode(message(), bytes, size);
	}

	inline void response(
		IN const std::string& msg,
		IN uint16_t iid,
		IN bool is_last = true,
		IN MessageCategory category = category_general)
	{
		StringCodec codec(msg);
		MessageHandlerT<std::string>::response(codec, iid, is_last, category);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif