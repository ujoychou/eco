#pragma once
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
#include <eco/rx/RxExport.h>
#include <eco/net/codec/Codec.h>
#include <eco/net/ProtocolTcp.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
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
	// option: is last of array response
	option_last				= 0x01,
	// option: message type to identify message
	option_type				= 0x02,
	// option: request data remote peer will send back
	option_data				= 0x04,
	// option: error message
	option_error			= 0x08,
	// option: topic snap
	option_snap				= 0x10,
	// option: topic stamp
	option_stamp			= 0x80,
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
	uint64_t		m_request_data;
	uint16_t		m_topic_snap;
	uint16_t		m_topic_stamp;
	Codec*			m_codec;

public:
	inline MessageMeta()
	{
		memset(this, 0, sizeof(*this));
		m_category = category_message;
	}

	inline MessageMeta& sync(bool is)
	{
		eco::set(m_category, category_sync, is);
		return *this;
	}
	inline bool sync() const
	{
		return eco::has(m_category, category_sync);
	}

	inline MessageMeta& error(bool v)
	{
		eco::set(m_option, option_error, v);
		return *this;
	}
	inline bool error() const
	{
		return eco::has(m_option, option_error);
	}

	inline MessageMeta& last(bool is)
	{
		eco::set(m_option, option_last, is);
		return *this;
	}
	inline bool last() const
	{
		return eco::has(m_option, option_last);
	}

	inline MessageMeta& category(MessageCategory v)
	{
		eco::add(m_category, v);
		return *this;
	}

	inline MessageMeta& encrypt(bool is)
	{
		eco::set(m_category, category_encrypted, is);
		return *this;
	}

	inline MessageMeta& check_sum(bool is)
	{
		eco::set(m_category, category_checksum, is);
		return *this;
	}

	inline MessageMeta& message_type(uint32_t v)
	{
		m_message_type = v;
		eco::set(m_option, option_type, v != 0);
		return *this;
	}

	inline MessageMeta& request_data(void* v)
	{
		request_data((uint64_t)v);
		return *this;
	}
	inline MessageMeta& request_data(uint64_t v)
	{
		m_request_data = v;
		eco::set(m_option, option_data, v != 0);
		return *this;
	}

	inline MessageMeta& codec(Codec& v)
	{
		m_codec = &v;
		return *this;
	}
};


////////////////////////////////////////////////////////////////////////////////
class Protocol : public eco::RxHeap
{
public:
	/*@ get message version.*/
	uint8_t version = 0;
	/*@ max body size that this protocol support.*/
	uint32_t maxsize = 0;

	/* encode a heartbeat message, heartbeat encoded by protocol head, not by
	protocol, so it has no protocol info. and heartbeat don't judge protocol,
	it's responsibility is only judge peer live status.
	*/
	virtual void encode_heartbeat(
		eco::String& bytes) const = 0;

	/*@ encode message to bytes string.*/
	virtual bool encode(
		eco::String& bytes,
		uint32_t& start,
		const eco::net::MessageMeta& meta) = 0;

	/*@ get message body size that don't include head size.*/
	virtual bool decode_tcp(
		eco::net::MessageTcp& head,
		const char* bytes,
		uint32_t size) const = 0;

	/*@ get message meta and message bytes.*/
	virtual bool decode_meta(
		eco::net::MessageMeta& meta,
		eco::Bytes& data,
		eco::String& bytes,
		uint32_t head_size) = 0;

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
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net)
ECO_NS_END(eco)