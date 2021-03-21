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
	// option: error response.
	option_error			= 0x40,
	// option: request data that server will send back.
	option_data				= 0x08,
	// option: message has a session id.
	option_sess				= 0x80,
};
typedef uint8_t MessageOption;


////////////////////////////////////////////////////////////////////////////////
class MessageMeta
{
public:
	MessageCategory	m_category;
	MessageModel	m_model;
	MessageOption   m_option;

	// meta option data.
	uint32_t		m_message_type;
	uint64_t		m_option_data;
	SessionId		m_session_id;
	Codec*			m_codec;

public:
	inline MessageMeta()
	{
		memset(this, 0, sizeof(*this));
		m_category = category_message;
	}

	inline MessageMeta& sync(IN bool is = true)
	{
		eco::set(m_category, category_sync, is);
		return *this;
	}
	inline bool is_sync() const
	{
		return eco::has(m_category, category_sync);
	}

	inline MessageMeta& category(IN MessageCategory v)
	{
		eco::add(m_category, v);
		return *this;
	}

	inline MessageMeta& encrypt(IN bool is = true)
	{
		eco::set(m_category, category_encrypted, is);
		return *this;
	}

	inline MessageMeta& check_sum(IN bool is = true)
	{
		eco::set(m_category, category_checksum, is);
		return *this;
	}

	inline MessageMeta& session_id(IN SessionId id)
	{
		m_session_id = id;
		eco::set(m_option, option_sess, m_session_id != none_session);
		return *this;
	}

	inline MessageMeta& message_type(IN uint32_t v)
	{
		m_message_type = v;
		eco::set(m_option, option_type, v != 0);
		return *this;
	}

	inline bool has_error() const
	{
		return eco::has(m_option, option_error);
	}

	inline MessageMeta& error(IN bool v)
	{
		eco::set(m_option, option_error, v);
		return *this;
	}

	inline MessageMeta& last(IN bool is = true)
	{
		eco::set(m_option, option_last, is);
		return *this;
	}
	inline bool is_last() const
	{
		return eco::has(m_option, option_last);
	}

	inline MessageMeta& option(IN const void* data)
	{
		if (sizeof(void*) == 4)
			set_request_data((uint32_t)(uint64_t)(data));
		else if (sizeof(void*) == 8)
			set_request_data((uint64_t)(data));
		return *this;
	}

	inline MessageMeta& snap(IN uint8_t v)
	{
		set_request_data(v);
		return *this;
	}

	inline MessageMeta& codec(IN Codec& cdc)
	{
		m_codec = &cdc;
		return *this;
	}

public:
	inline void set_request_data(IN const uint8_t req_data)
	{
		m_option_data = req_data;
		eco::add(m_option, option_req1);
		eco::add(m_option, option_data);
	}
	inline void set_request_data(IN const uint16_t req_data)
	{
		m_option_data = req_data;
		eco::add(m_option, option_req2);
		eco::add(m_option, option_data);
	}
	inline void set_request_data(IN const uint32_t req_data)
	{
		m_option_data = req_data;
		eco::add(m_option, option_req4);
		eco::add(m_option, option_data);
	}
	inline void set_request_data(IN const uint64_t req_data)
	{
		m_option_data = req_data;
		eco::add(m_option, option_req8);
		eco::add(m_option, option_data);
	}
	inline void set_request_data(
		IN const uint64_t req_data,
		IN const MessageOption opt)
	{
		m_option_data = req_data;
		eco::add(m_option, opt);
	}
	inline const uint8_t get_req1() const
	{
		return static_cast<uint8_t>(m_option_data);
	}
	inline const uint16_t get_req2() const
	{
		return static_cast<uint16_t>(m_option_data);
	}
	inline const uint32_t get_req4() const
	{
		return static_cast<uint32_t>(m_option_data);
	}
	inline const uint64_t get_req8() const
	{
		return m_option_data;
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
		IN  const eco::net::MessageMeta& meta) = 0;

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
		IN  uint32_t head_size) = 0;

public:
	// get size of "uint8_t".
	inline static uint8_t size_size_byte(uint8_t size)
	{
		if (size < 253)
			return 1;
		else if (size == 253)
			return 1 + 2;
		else if (size == 254)
			return 1 + 4;
		return 1 + 8;
	}

	// get size of data size.
	inline static uint8_t size_size(uint64_t size)
	{
		if (size < 253)
			return 1;
		else if (size <= max_uint16())
			return 1 + 2;
		else if (size <= max_uint32())
			return 1 + 4;
		return 1 + 8;
	}

	// decode size from bytes, and return size.
	template<typename int_t>
	inline static uint8_t size_decode(int_t& size, const char* bytes)
	{
		uint8_t flag = bytes[0];
		if (flag < 253)
		{
			size = flag;
			return 1;
		}
		else if (flag == 253)
		{
			size = (int_t)ntoh16(&bytes[1]);
			return 3;
		}
		else if (flag == 254)
		{
			size = (int_t)ntoh32(&bytes[1]);
			return 5;
		}
		size = (int_t)ntoh64(&bytes[1]);
		return 9;
	}

	// encode size to bytes.
	inline static void size_encode(eco::String& bytes, uint64_t size)
	{
		if (size < 253)
		{
			bytes.append(static_cast<uint8_t>(size));
		}
		else if (size <= max_uint16())
		{
			bytes.append(static_cast<uint8_t>(253));
			append_hton(bytes, static_cast<uint16_t>(size));
		}
		else if (size <= max_uint32())
		{
			bytes.append(static_cast<uint8_t>(254));
			append_hton(bytes, static_cast<uint32_t>(size));
		}
		else
		{
			bytes.append(static_cast<uint8_t>(255));
			append_hton(bytes, static_cast<uint64_t>(size));
		}
	}

	// encode size to bytes.
	inline static void size_encode(char* bytes, uint64_t size)
	{
		if (size < 253)
		{
			bytes[0] = static_cast<uint8_t>(size);
		}
		else if (size <= max_uint16())
		{
			bytes[0] = static_cast<uint8_t>(253);
			hton(&bytes[1], static_cast<uint16_t>(size));
		}
		else if (size <= max_uint32())
		{
			bytes[0] = static_cast<uint8_t>(254);
			hton(&bytes[1], static_cast<uint32_t>(size));
		}
		else
		{
			bytes[0] = static_cast<uint8_t>(255);
			hton(&bytes[1], static_cast<uint64_t>(size));
		}
	}

protected:
	virtual void encode_req(eco::String& bytes, const MessageMeta& meta) const
	{
		if (eco::has(meta.m_option, option_req1))
			bytes.append(static_cast<uint8_t>(meta.m_option_data));
		else if (eco::has(meta.m_option, option_req2))
			append_hton(bytes, static_cast<uint16_t>(meta.m_option_data));
		else if (eco::has(meta.m_option, option_req4))
			append_hton(bytes, static_cast<uint32_t>(meta.m_option_data));
		else if (eco::has(meta.m_option, option_req8))
			append_hton(bytes, meta.m_option_data);
	}

	virtual uint8_t decode_req(
		OUT MessageMeta& meta,
		IN  const char* bytes) const
	{
		if (eco::has(meta.m_option, option_req1))
		{
			meta.m_option_data = bytes[0];
			return 1;
		}
		else if (eco::has(meta.m_option, option_req2))
		{
			meta.m_option_data = ntoh16(&bytes[0]);
			return sizeof(uint16_t);
		}
		else if (eco::has(meta.m_option, option_req4))
		{
			meta.m_option_data = ntoh32(&bytes[0]);
			return sizeof(uint32_t);
		}
		else if (eco::has(meta.m_option, option_req8))
		{
			meta.m_option_data = ntoh64(&bytes[0]);
			return sizeof(uint64_t);
		}
		return 0;
	}

	virtual uint8_t size_req(const MessageMeta& meta) const
	{
		if (eco::has(meta.m_option, option_req1))
			return sizeof(uint8_t);
		else if (eco::has(meta.m_option, option_req2))
			return sizeof(uint16_t);
		else if (eco::has(meta.m_option, option_req4))
			return sizeof(uint32_t);
		else if (eco::has(meta.m_option, option_req8))
			return sizeof(uint64_t);
		return 0;
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif