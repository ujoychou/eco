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
	// option: message has a session id.
	option_sess				= 0x80,
};
typedef uint8_t MessageOption;


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

	inline void set_message_type(IN const uint32_t type)
	{
		m_message_type = type;
		eco::add(m_option, option_type);
	}

	inline void set_last(IN bool is)
	{
		eco::set(m_option, option_last, is);
	}
	inline bool last() const
	{
		return eco::has(m_option, option_last);
	}

public:
	inline void set_request_data(IN const void* req_data)
	{
		if (sizeof(void*) == 4)
			set_request_data((uint32_t)(uint64_t)(req_data));
		else if (sizeof(void*) == 8)
			set_request_data((uint64_t)(req_data));
	}
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
		IN const MessageOption opt)
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