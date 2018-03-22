#ifndef ECO_NET_TCP_PROTOCOL_H
#define ECO_NET_TCP_PROTOCOL_H
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
#include <eco/ExportApi.h>
#include <eco/net/protocol/Protocol.h>
#include <eco/net/protocol/Crypt.h>
#include <eco/net/protocol/Check.h>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class TcpProtocolHead : public ProtocolHead
{
private:
	friend class TcpProtocol;

	// "protocol head data" class.
	struct __Data__
	{
		// message version.
		uint8_t		m_version;

		// message category.
		uint8_t		m_category;

		// message bytes size.
		uint16_t	m_size;
	};

	// "message head" value.
	enum
	{
		size_head			= 4,

		// head pos.
		pos_version			= 0,
		pos_category		= 1,
		pos_size			= 2,
	};

public:
	virtual uint32_t size() const override
	{
		return size_head;
	}

	virtual uint32_t max_data_size() const override
	{
		return 0xFFFF;	// 65535;
	}

	virtual bool decode_data_size(
		OUT uint32_t& data_size,
		IN const char* bytes,
		IN const uint32_t size,
		IN  eco::Error& e) const override
	{
		data_size = ntoh16(bytes + pos_size);
		return true;
	}

	virtual bool decode(
		OUT eco::net::MessageHead& head,
		IN  const eco::String& bytes,
		IN  eco::Error& e) const override
	{
		head.m_version = bytes[pos_version];
		head.m_category = bytes[pos_category];
		if (!eco::has(head.m_category, category_message) &&
			!eco::has(head.m_category, category_heartbeat))
		{
			e.id(e_message_category) << "message category is invalid: "
				<< head.m_category;
			return false;
		}
		return true;
	}

	virtual void encode_heartbeat(
		OUT eco::String& bytes) const
	{
		MessageHead head;
		head.m_version = -1;
		head.m_category = category_heartbeat;
		encode_append(bytes, head);
	}

public:
	inline uint32_t encode_data_size(OUT eco::String& bytes) const
	{
		hton(&bytes[pos_size], (uint16_t)(bytes.size() - size_head));
		return 0;
	}

	inline void encode_append(
		OUT eco::String& bytes,
		IN  const eco::net::MessageHead& head) const
	{
		bytes.append(size_head, 0);
		bytes[pos_version] = static_cast<char>(head.m_version);
		bytes[pos_category] = static_cast<char>(head.m_category);
	}
};


////////////////////////////////////////////////////////////////////////////////
class TcpProtocol : public Protocol
{
private:
	// "message data" class.
	struct __Data__
	{
		// message model.
		uint8_t		m_model;

		// message option.
		uint8_t		m_option;

		// optional field: identity the session.
		uint32_t	m_session_id;

		// message type.
		uint16_t	m_type;

		// optional field: the request data from client.
		uint32_t	m_request_data;
	};

	// "message data" value
	enum
	{
		// #.parameter head
		size_model			= 1,
		size_option			= 1,
		// #.parameter option data.
		size_session_id		= 4,
		size_type			= 2,
		size_req4			= 4,
		size_req8			= 8,
	};

	inline uint32_t get_model_pos(IN  const uint32_t head_size) const
	{
		return head_size;
	}
	inline uint32_t get_option_pos(IN  const uint32_t head_size) const
	{
		return head_size + size_model;
	}
	inline uint32_t get_option_data_pos(IN  const uint32_t head_size) const
	{
		return get_option_pos(head_size) + size_option;
	}

	inline uint32_t get_meta_size(IN const MessageMeta& meta) const
	{
		uint32_t pos = size_model + size_option;
		if (eco::has(meta.m_option, option_sess))
		{
			pos += size_session_id;
		}
		if (eco::has(meta.m_option, option_type))
		{
			pos += size_type;
		}
		if (eco::has(meta.m_option, option_req4))
		{
			pos += size_req4;
		}
		else if (eco::has(meta.m_option, option_req8))
		{
			pos += size_req8;
		}
		return pos;
	}


///////////////////////////////////////////////////////////////////////// DECODE
public:
	TcpProtocol() : m_crypt(nullptr), m_check(nullptr)
	{}

	virtual uint32_t version() override
	{
		return 1;
	}

	virtual bool decode(
		OUT eco::net::MessageMeta& meta,
		OUT eco::Bytes& data,
		IN  eco::String& bytes,
		IN  eco::Error& e) override
	{
		// check sum message.
		uint32_t check_sum_size = 0;
		uint32_t head_size = eco::net::TcpProtocolHead::size_head;
		if (eco::has(meta.m_category, category_checksum) && m_check != nullptr)
		{
			if (!m_check->decode(bytes))
			{
				e.id(e_message_checksum)
					<< "checksum bytes size error or checksum match fail.";
				return false;
			}
			check_sum_size = m_check->get_byte_size();
		}
		// decrypt message.
		if (eco::has(meta.m_category, category_encrypted) && m_crypt != nullptr)
		{
			uint32_t crypt_size = bytes.size();
			crypt_size -= (head_size + check_sum_size);
			bytes = m_crypt->decode(bytes, head_size, crypt_size, e);
			if (bytes.null())
			{
				return false;
			}
		}

		// get model & option.
		uint32_t opt_data_pos = get_option_data_pos(head_size);
		if (bytes.size() < opt_data_pos)
		{
			e.id(e_protocol_parameter)
				<< "message size is too smaller to have option: "
				<< bytes.size() << '<' << opt_data_pos;
			return false;
		}
		meta.m_model = MessageModel(bytes[get_model_pos(head_size)]);
		meta.m_option = MessageOption(bytes[get_option_pos(head_size)]);

		// get option data.
		uint32_t meta_end = get_meta_size(meta) + head_size;
		if (bytes.size() < meta_end)
		{
			e.id(e_protocol_parameter)
				<< "message size is too small to have meta: "
				<< bytes.size() << '<' << meta_end;
			return false;
		}
		uint32_t pos = opt_data_pos;
		if (eco::has(meta.m_option, option_sess))
		{
			ntoh(meta.m_session_id, pos, &bytes[pos]);
		}
		if (eco::has(meta.m_option, option_type))
		{
			uint16_t msg_type = 0;
			ntoh(msg_type, pos, &bytes[pos]);
			meta.m_message_type = msg_type;
		}
		if (eco::has(meta.m_option, option_req4))
		{
			uint32_t req4 = 0;
			ntoh(req4, pos, &bytes[pos]);
			meta.m_request_data = req4;
		}
		else if (eco::has(meta.m_option, option_req8))
		{
			ntoh(meta.m_request_data, pos, &bytes[pos]);
		}

		// message data bytes.
		data.m_data = &bytes[pos];
		data.m_size = bytes.size() - pos - check_sum_size;
		return true;
	}

	virtual bool encode(
		OUT eco::String& bytes,
		OUT uint32_t& start,
		IN  const eco::net::MessageMeta& meta,
		OUT eco::Error& e) override
	{
		assert(meta.m_codec != nullptr);

		// 1.init bytes size.
		eco::net::TcpProtocolHead prot_head;
		uint16_t head_size = prot_head.size();				// @head size.
		uint32_t byte_size = get_meta_size(meta);			// #@meta size.
		uint32_t code_size = meta.m_codec->get_byte_size();	// #@message size.	
		byte_size += code_size;
		if (eco::has(meta.m_category, category_encrypted) && m_crypt != nullptr)
		{
			byte_size = m_crypt->get_byte_size(byte_size);	// [#]@crypt size.
		}
		byte_size += head_size;
		if (eco::has(meta.m_category, category_checksum) && m_check != nullptr)
		{
			byte_size += m_check->get_byte_size();			// [@]checksum size.
		}
		bytes.clear();
		bytes.reserve(byte_size);

		// 2.init message version and category.
		eco::net::MessageHead head;
		head.m_version = version();
		head.m_category = meta.m_category;
		prot_head.encode_append(bytes, head);

		// 3.init message type and optional data.
		bytes.append(static_cast<char>(meta.m_model));
		bytes.append(static_cast<char>(meta.m_option));
		if (eco::has(meta.m_option, option_sess))
		{
			append_hton(bytes, meta.m_session_id);
		}
		if (eco::has(meta.m_option, option_type))
		{
			append_hton(bytes, static_cast<uint16_t>(meta.m_message_type));
		}
		if (eco::has(meta.m_option, option_req4))
		{
			append_hton(bytes, static_cast<uint32_t>(meta.m_request_data));
		}
		else if (eco::has(meta.m_option, option_req8))
		{
			append_hton(bytes, meta.m_request_data);
		}

		// 5.encode message object.
		meta.m_codec->encode_append(bytes, code_size);

		// 6.encrypt message.
		if (eco::has(head.m_category, category_encrypted) && m_crypt != nullptr)
		{
			bytes = m_crypt->encode(bytes, head_size);
			if (bytes.null())
			{
				e.id(e_message_encode) << "crypt message fail.";
				return false;
			}
		}

		// 7.append checksum.
		if (eco::has(head.m_category, category_checksum) && m_check != nullptr)
		{
			m_check->encode(bytes);
		}

		// 8.reset bytes size.
		start = prot_head.encode_data_size(bytes);
		return true;
	}

private:
	Crypt* m_crypt;
	Check* m_check;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif