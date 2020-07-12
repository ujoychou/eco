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
#include <eco/net/protocol/ProtocolVersion.h>
#include <eco/Object.h>



namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
enum
{
	// model bit3: ftdc & logging.
	req		= 1,
	rsp		= 2,
	sub		= 3,
	pub		= 4,

	// req_data bit2: req1/req2/req4/req8
	req1	= 1,
	req2	= 2,
	req4	= 3,
	req8	= 4,

	// bit3 left.
};
// TODO: move req_data option; model fixed size; option opt size.
typedef uint8_t MessageModel;


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
	option_req1				= 0x10,
	option_req2				= 0x20,
	// option: error id response.
	option_error			= 0x40,
	// option: message has a session id.
	option_sess				= 0x80,
};
typedef uint8_t MessageOptionMeta;


////////////////////////////////////////////////////////////////////////////////
class MessageOption
{
public:
	MessageCategory	m_category;

	// meta option data.
	uint32_t		m_error_id;
	uint32_t		m_session_id;
	uint32_t		m_message_type;
	uint64_t		m_request_data;

	MessageOptionMeta	m_option;

public:
	inline MessageOption()
		: m_category(category_message)
		, m_error_id(0)
		, m_session_id(none_session)
		, m_message_type(0)
		, m_request_data(0)
		, m_option(0)
	{}

	inline MessageOption(
		IN uint32_t msg_type,
		IN uint32_t err_id = 0,
		IN bool     last_msg = true,
		IN uint32_t sess_id = none_session)
	{
		m_category = category_message;
		m_request_data = 0;
		m_option = 0;
		last(last_msg);
		error_id(err_id);
		session_id(sess_id);
		message_type(msg_type);
	}

	inline MessageOption& sync(IN bool is = true)
	{
		eco::set(m_category, category_sync, is);
		return *this;
	}
	inline bool is_sync() const
	{
		return eco::has(m_category, category_sync);
	}

	inline MessageOption& category(IN MessageCategory v)
	{
		eco::add(m_category, v);
		return *this;
	}

	inline MessageOption& encrypt(IN bool is = true)
	{
		eco::set(m_category, category_encrypted, is);
		return *this;
	}

	inline MessageOption& check_sum(IN bool is = true)
	{
		eco::set(m_category, category_checksum, is);
		return *this;
	}

	inline MessageOption& session_id(IN uint32_t id)
	{
		m_session_id = id;
		eco::set(m_option, option_sess, m_session_id != none_session);
		return *this;
	}

	inline MessageOption& message_type(IN uint32_t v)
	{
		m_message_type = v;
		eco::set(m_option, option_type, v != 0);
		return *this;
	}

	inline bool has_error() const
	{
		return m_error_id != 0;
	}

	inline MessageOption& error_id(IN uint32_t v)
	{
		m_error_id = v;
		eco::set(m_option, option_error, v != 0);
		return *this;
	}

	inline MessageOption& last(IN bool is = true)
	{
		eco::set(m_option, option_last, is);
		return *this;
	}
	inline bool is_last() const
	{
		return eco::has(m_option, option_last);
	}

	inline MessageOption& option(IN const void* data)
	{
		if (sizeof(void*) == 4)
			set_request_data((uint32_t)(uint64_t)(data));
		else if (sizeof(void*) == 8)
			set_request_data((uint64_t)(data));
		return *this;
	}

	inline MessageOption& snap(IN uint8_t v)
	{
		set_request_data(v);
		return *this;
	}

public:
	inline void set_request_data(IN const uint8_t req_data)
	{
		m_request_data = req_data;
		eco::add(m_option, option_req1);
	}
	inline void set_request_data(IN const uint16_t req_data)
	{
		m_request_data = req_data;
		eco::add(m_option, option_req2);
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
	inline void set_request_data(
		IN const uint64_t req_data,
		IN const MessageOptionMeta opt)
	{
		m_request_data = req_data;
		eco::add(m_option, opt);
	}
	inline const uint8_t get_req1() const
	{
		return static_cast<uint8_t>(m_request_data);
	}
	inline const uint16_t get_req2() const
	{
		return static_cast<uint16_t>(m_request_data);
	}
	inline const uint32_t get_req4() const
	{
		return static_cast<uint32_t>(m_request_data);
	}
	inline const uint64_t get_req8() const
	{
		return m_request_data;
	}
};


////////////////////////////////////////////////////////////////////////////////
class MessageMeta : public MessageOption
{
public:
	MessageModel		m_model;
	Codec*				m_codec;
	
public:
	inline MessageMeta() : m_model(0), m_codec(0)
	{}

	inline MessageMeta(IN Codec* c, const MessageOption& opt)
		: MessageOption(opt), m_model(0), m_codec(c)
	{}

	inline MessageMeta& codec(IN Codec& cdc)
	{
		m_codec = &cdc;
		return *this;
	}

	inline MessageMeta& operator=(IN const MessageOption& opt)
	{
		((MessageOption&)*this) = opt;
		return *this;
	}
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Protocol : public ProtocolVersion
{
	ECO_OBJECT_API(Protocol);
public:
	/*@ get message version.*/
	void set_version(uint8_t);
	uint8_t version() const;

	/*@ max body size that this protocol support.*/
	void set_max_size(uint32_t);
	uint32_t max_size() const;

	/* encode a heartbeat message, heartbeat encoded by protocol head, not by
	protocol, so it has no protocol info. and heartbeat don't judge protocol,
	it's responsibility is only judge peer live status.
	*/
	virtual void encode_heartbeat(
		OUT eco::String& bytes) const = 0;

	/*@ encode message to bytes string.*/
	virtual bool encode(
		OUT eco::String& bytes,
		OUT uint32_t& start,
		IN  const eco::net::MessageMeta& meta,
		OUT eco::Error& e) = 0;

	/*@ get message body size that don't include head size.*/
	virtual bool decode_size(
		OUT IN eco::net::MessageHead& head,
		IN const char* bytes,
		IN const uint32_t size) const = 0;

	/*@ get message meta and message bytes.*/
	virtual bool decode_meta(
		OUT eco::net::MessageMeta& meta,
		OUT eco::Bytes& data,
		IN  eco::String& bytes,
		IN  uint32_t head_size,
		IN  eco::Error& e) = 0;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif