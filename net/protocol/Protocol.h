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
#include <eco/net/protocol/Codec.h>
#include <eco/net/protocol/ProtocolHead.h>



namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
enum
{
	// ftdc; logging;
	req				= 0x01,
	rsp				= 0x02,
	sub				= 0x04,
	pub				= 0x08,
	all				= 0x10,
	set				= 0x20,
	str				= 0x40,

	// object all.(no using current, only for understand topic.)
	// case: tid = (product_type, 0, 0), all product.
	sub_all_req		= sub | all | req,
	sub_all_rsp		= sub | all | rsp,
	pub_all			= pub | all,

	/* object set: type + property condition set.
	case: product of the user whose id=2.
	tid = (product_type, userid, 2), 
	case: it is a stock product of the user whose id=2.
	tid = (product_type, userid, 2, type, stock)
	this case can't be support by current topic id, it can solve by solution:
	tid = (product_type, data_privilege, 1)
	data_privilege = 1 means {data_type=product_type, userid=2, type=stock;}
	*/
	sub_set_req		= sub | set | req,
	sub_set_rsp		= sub | set | rsp,
	pub_set			= pub | set,
	// object set & all. topic id = "string topic id";
	sub_str_req		= sub | str | req,
	sub_str_rsp		= sub | str | rsp,
	pub_str			= pub | str,
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
	// option: message has a session id.
	option_sess				= 0x10,
	// option: auto authority(support for auto login).
	//option_auto			= 0x20,
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
	// meta coder.
	Codec*			m_codec;
	
public:
	inline MessageMeta()
	{
		memset(this, 0, sizeof(*this));
	}

	inline MessageMeta(IN MessageCategory v)
	{
		memset(this, 0, sizeof(*this));
		m_category = v;
	}

	inline MessageMeta(
		IN Codec& codec,
		IN const uint32_t session_id,
		IN const uint32_t type,
		IN const bool encrypted)
	{
		memset(this, 0, sizeof(*this));
		m_category = category_message;
		if (encrypted)
			eco::add(m_category, category_encrypted);
		if (session_id != none_session)
			eco::add(m_category, category_session);
		set_session_id(session_id);
		set_message_type(type);
		m_codec = &codec;
	}

	inline void set_session_id(IN const uint32_t id)
	{
		m_session_id = id;
		eco::set(m_option, option_sess, m_session_id != none_session);
	}

	inline void set_last(IN const bool is)
	{
		eco::set(m_option, option_last, is);
	}
	inline const bool last() const
	{
		return eco::has(m_option, option_last);
	}

	inline void set_message_type(IN const uint32_t type)
	{
		m_message_type = type;
		eco::add(m_option, option_type);
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
			set_request_data((uint32_t)(uint64_t)(req_data));
		else if (sizeof(void*) == 8)
			set_request_data((uint64_t)(req_data));
	}
	inline void set_request_data(IN uint64_t req_data, IN uint32_t opt)
	{
		m_request_data = req_data;
		eco::set(m_option, option_req4, eco::has(opt, option_req4));
		eco::set(m_option, option_req8, eco::has(opt, option_req8));
	}
	inline const uint32_t get_req4() const
	{
		return static_cast<uint32_t>(m_request_data);
	}
	inline const uint64_t get_req8() const
	{
		return m_request_data;
	}

public:
	inline bool model_topic() const
	{
		return eco::has(m_model, sub | pub);
	}
	inline bool model_topic_all() const
	{
		return eco::has(m_model, all);
	}
	inline bool model_topic_set() const
	{
		return eco::has(m_model, set);
	}
	inline bool model_topic_str() const
	{
		return eco::has(m_model, str);
	}
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Protocol : public eco::HeapOperators
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
		IN  eco::Error& e) = 0;

	/*@ #1.encode message to bytes string.*/
	virtual bool encode(
		OUT eco::String& bytes,
		OUT uint32_t& start,
		IN  const eco::net::MessageMeta& meta,
		OUT eco::Error& e) = 0;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif