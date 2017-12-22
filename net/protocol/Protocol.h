#ifndef ECO_NET_PROTOCOL_H
#define ECO_NET_PROTOCOL_H
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
#include <eco/net/SessionData.h>
#include <eco/net/protocol/Codec.h>
#include <eco/net/protocol/ProtocolHead.h>



namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
enum
{
	model_none				= 0x00,
	// req/rsp mode.
	model_req				= 0x01,
	model_rsp				= 0x02,

	// object all.
	// case: tid = (product_type, 0, 0), all product.
	model_sub_all_req		= 0x03,
	model_sub_all_rsp		= 0x04,
	model_pub_all			= 0x05,
	/* object set: type + property condition set.
	case: product of the user whose id=2.
	tid = (product_type, userid, 2), 
	case: it is a stock product of the user whose id=2.
	tid = (product_type, userid, 2, type, stock)
	this case can't be support by current topic id, it can solve by solution:
	tid = (product_type, data_privilege, 1)
	data_privilege = 1 means {data_type=product_type, userid=2, type=stock;}
	*/
	model_sub_set_req		= 0x06,
	model_sub_set_rsp		= 0x07,
	model_pub_set			= 0x08,
	// object set & all. topic id = "string topic id";
	model_sub_str_req		= 0x09,
	model_pub_str_rsp		= 0x0A,
	model_pub_str			= 0x0B,
};
// for user define: MessageCategory is uint16_t.
typedef uint16_t MessageModel;


////////////////////////////////////////////////////////////////////////////////
// message option for message to send.
enum
{
	// option: is last array data.
	option_last				= 0x01,
	// option: has message type to identify message.
	option_type				= 0x02,
	// option: request data that server will send back.
	option_req4				= 0x04,
	option_req8				= 0x08,
};
// for user define: MessageOption is uint16_t.
typedef uint32_t MessageOption;


////////////////////////////////////////////////////////////////////////////////
class MessageMeta
{
public:
	MessageCategory	m_category;
	// meta fixed head.
	MessageModel	m_model;
	MessageOption	m_option;
	// meta option data.
	uint32_t		m_session_id;
	uint32_t		m_message_type;
	uint64_t		m_request_data;
	// meta option data topic.
	uint16_t		m_topic_type;
	uint16_t		m_topic_prop;
	uint64_t		m_topic_value;
	eco::Bytes		m_topic_str;
	// meta coder.
	Codec*			m_codec;

public:
	inline MessageMeta()
	{
		memset(this, 0, sizeof(*this));
	}

	inline MessageMeta(
		IN Codec& codec,
		IN const uint32_t session_id,
		IN const uint32_t type,
		IN const MessageModel model,
		IN const MessageCategory category = category_message)
	{
		m_category = category;
		set_session_id(session_id);
		set_message_type(type);
		m_model = model;
		m_codec = &codec;
	}

	inline void set_session_id(IN const uint32_t id)
	{
		m_session_id = id;
		eco::set(m_category, category_session_mode,
			m_session_id != none_session);
	}

	inline void set_message_type(IN const uint32_t type)
	{
		m_message_type = type;
		eco::add(m_category, option_type);
	}

	inline void set_request_data(IN const uint32_t req_data)
	{
		m_request_data = req_data;
		eco::add(m_option, option_req4);
	}
	inline void set_request_data(IN const uint64_t req_data)
	{
		m_request_data = req_data;
		eco::add(m_option, option_req8);
	}
	inline void set_request_data(IN const void* req_data)
	{
		if (sizeof(void*) == 4)
			set_request_data(reinterpret_cast<uint32_t>(req_data));
		else if (sizeof(void*) == 8)
			set_request_data(reinterpret_cast<uint64_t>(req_data));
	}

	inline bool model_topic() const
	{
		return model_sub_all_req <= m_model && m_model <= model_pub_str;
	}
	inline bool model_topic_all() const
	{
		return model_sub_all_req <= m_model && m_model <= model_pub_all;
	}
	inline bool model_topic_set() const
	{
		return model_sub_set_req <= m_model && m_model <= model_pub_set;
	}
	inline bool model_topic_str() const
	{
		return model_sub_str_req <= m_model && m_model <= model_pub_str;
	}
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Protocol
{
	ECO_OBJECT(Protocol);
public:
	inline Protocol() {}

	/*@ #2.get message version.*/
	virtual uint32_t version() = 0;

	/*@ #3.get message meta and message bytes.*/
	virtual bool decode(
		OUT eco::net::MessageMeta& meta,
		OUT eco::Bytes& data,
		IN  eco::String& bytes,
		IN  eco::net::ProtocolHead& prot_head,
		IN  eco::Error& e) = 0;

	/*@ #1.encode message to bytes string.*/
	virtual bool encode(
		OUT eco::String& bytes,
		IN  const eco::net::MessageMeta& meta,
		IN  eco::net::ProtocolHead& prot_head,
		OUT eco::Error& e) = 0;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif