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
	virtual uint32_t head_size() const override
	{
		return size_head;
	}

	virtual uint32_t decode_data_size(IN const char* bytes) const override
	{
		return ntoh16(bytes + pos_size);
	}

	virtual void encode_data_size(OUT eco::String& bytes) const override
	{
		hton(&bytes[pos_size], static_cast<uint16_t>(bytes.size() - size_head));
	}

public:
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

	virtual void encode_append(
		OUT eco::String& bytes,
		IN  const eco::net::MessageHead& head) const override
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
		// optional field: identity the session.
		uint32_t	m_session_id;

		// optional field: identity the request from client.
		uint32_t	m_request_id;

		// message type.
		uint16_t	m_type;

		// message model.
		uint8_t		m_model;

		// message option.
		uint8_t		m_option;
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
		// #.parameter topic
		size_topic_type		= 2,
		size_topic_prop		= 2,
		size_topic_value	= 8,

		size_topic_all		= size_topic_type,
		size_topic_set		= size_topic_type + 
							  size_topic_prop + size_topic_value,
		size_topic_str		= 1,
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

	inline uint32_t get_meta_end(
		IN const MessageMeta& meta,
		IN  const uint32_t head_size) const
	{
		uint32_t pos = get_option_data_pos(head_size);
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
		
		if (meta.model_topic_all())
		{
			pos += size_topic_all;
		}
		else if (meta.model_topic_set())
		{
			pos += size_topic_set;
		}
		else if (meta.model_topic_str())
		{
			pos += 1; // topic str length.
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
		IN  const uint32_t head_size,
		IN  eco::Error& e) override
	{
		// check sum message.
		uint32_t check_sum_size = 0;
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
		uint32_t meta_end = get_meta_end(meta, head_size);
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
		
		// get subscription topic info.
		if (meta.model_topic_all())
		{
			ntoh(meta.m_topic_type, pos, &bytes[pos]);
		}
		else if (meta.model_topic_set())
		{
			ntoh(meta.m_topic_type, pos, &bytes[pos]);
			ntoh(meta.m_topic_prop, pos, &bytes[pos]);
			ntoh(meta.m_topic_value, pos, &bytes[pos]);
		}
		else if (meta.model_topic_str())
		{
			// validate topic str_len size.
			meta.m_topic_str.m_size = bytes[pos++];
			meta.m_topic_str.m_data = &bytes[pos];
			pos += meta.m_topic_str.m_size;
			// validate topic str size.
			if (bytes.size() < pos + 1)
			{
				e.id(e_protocol_parameter)
					<< "message size is smaller than meta topic str size: "
					<< bytes.size() << '<' << pos + 1;
				return false;
			}
		}

		// message data bytes.
		data.m_data = &bytes[pos];
		data.m_size = bytes.size() - pos - check_sum_size;
		return true;
	}

	virtual bool encode(
		OUT eco::String& bytes,
		IN  const eco::net::MessageMeta& meta,
		IN  eco::net::ProtocolHead& prot_head,
		OUT eco::Error& e) override
	{
		assert(meta.m_codec != nullptr);

		// 1.init bytes size.
		uint32_t byte_size = get_meta_end(meta, prot_head.head_size());
		uint32_t code_size = meta.m_codec->get_byte_size();	// meta size.
		if (meta.m_topic_str.m_size > 0)	// topic str size.
			byte_size += meta.m_topic_str.m_size;
		byte_size += (m_crypt == nullptr)	// crypt size.
			? code_size : m_crypt->get_byte_size(code_size);
		if (m_check != nullptr)				// checksum size.
			byte_size += m_check->get_byte_size();
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

		// 4.init message topic.
		if (meta.model_topic_all())
		{
			append_hton(bytes, meta.m_topic_type);
		}
		else if (meta.model_topic_set())
		{
			append_hton(bytes, meta.m_topic_type);
			append_hton(bytes, meta.m_topic_prop);
			append_hton(bytes, meta.m_topic_value);
		}
		else if (meta.model_topic_str())
		{
			bytes.append(meta.m_topic_str);
		}

		// 5.encode message object.
		meta.m_codec->encode_append(bytes, code_size);

		// 6.encrypt message.
		if (eco::has(head.m_category, category_encrypted) && m_crypt != nullptr)
		{
			bytes = m_crypt->encode(bytes, TcpProtocolHead::size_head);
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
		prot_head.encode_data_size(bytes);
		return true;
	}

private:
	Crypt* m_crypt;
	Check* m_check;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif