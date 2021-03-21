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
#ifndef ECO_PROTOBUF2
#include <google/protobuf/util/json_util.h>
#endif

#ifndef ECO_AUTO_LINK_NO
#	pragma comment(lib, "libprotobuf.lib")
#endif


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
class NullRequest;
////////////////////////////////////////////////////////////////////////////////
class ProtobufCodec : public eco::net::Codec
{
public:
	inline ProtobufCodec(const void* msg = nullptr) : eco::net::Codec(msg)
	{}

	const google::protobuf::Message& message() const
	{
		return *(google::protobuf::Message*)m_msg;
	}

	virtual void set_message(void* message) override
	{
		m_msg = static_cast<google::protobuf::Message*>(message);
	}

	virtual uint32_t byte_size() const override
	{
		return message().ByteSize();
	}

	virtual void encode(OUT char* bytes, IN uint32_t size) const override
	{
		message().SerializeToArray(bytes, (int)size);
	}

	virtual void* decode(IN const char* bytes, IN uint32_t size) override
	{
		auto& msg = *(google::protobuf::Message*)m_msg;
		return msg.ParseFromArray(bytes, (int)size) ? m_msg : nullptr;
	}
};


////////////////////////////////////////////////////////////////////////////////
inline std::string to_json(const google::protobuf::Message& msg)
{
	std::string json;
#ifndef ECO_PROTOBUF2
	google::protobuf::util::MessageToJsonString(msg, &json);
#endif
	return std::move(json);
}
inline std::string to_json(const NullRequest&)
{
	return eco::empty_str;
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#endif
#endif