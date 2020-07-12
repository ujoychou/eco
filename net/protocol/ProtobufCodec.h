#ifndef ECO_NET_PROTOBUF_CODEC_H
#define ECO_NET_PROTOBUF_CODEC_H
#ifndef ECO_NO_PROTOBUF
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
#include <eco/net/protocol/Codec.h>
#include <google/protobuf/message.h>

#ifndef ECO_AUTO_LINK_NO
#	pragma comment(lib, "libprotobuf.lib")
#endif

namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class ProtobufCodec : public eco::net::Codec
{
public:
	inline ProtobufCodec() : m_msg(nullptr)
	{}

	inline explicit ProtobufCodec(IN const google::protobuf::Message& msg)
	{
		m_msg = const_cast<google::protobuf::Message*>(&msg);
	}

	virtual void set_message(void* message) override
	{
		m_msg = static_cast<google::protobuf::Message*>(message);
	}

	virtual uint32_t byte_size() const override
	{
		return m_msg->ByteSize();
	}

	virtual void encode(OUT char* bytes, IN uint32_t size) const override
	{
		m_msg->SerializeToArray(bytes, (int)size);
	}

	virtual void* decode(IN const char* bytes, IN uint32_t size) override
	{
		return m_msg->ParseFromArray(bytes, (int)size) ? m_msg : nullptr;
	}

protected:
	google::protobuf::Message* m_msg;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif
#endif