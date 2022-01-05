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


ECO_NS_BEGIN(eco);
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpProtocol : public Protocol
{
protected:
	// "protocol head data" memory structure.
	struct __Head__
	{
		// message version.
		uint8_t		m_version;

		// message category.
		uint8_t		m_category;
	};

	// "message data" class.
	struct __Body__
	{
		// message bytes size.
		uint16_t	m_size;

		// message model.
		uint8_t		m_model;

		// message option.
		uint8_t		m_option;

		// optional field: identity the session.
		uint32_t	m_session_id;

		// message type.
		uint16_t	m_type;

		// optional field: the request data from client.
		uint64_t	m_option_data;
	};

	// "message data" value
	enum
	{
		// #.protocol ver.
		pos_version			= 0,
		pos_category		= 1,
		pos_size			= 2,

		// #.parameter head
		size_model			= 1,
		size_option			= 1,
		// #.parameter option data.
		size_session_id		= 4,
		size_type			= 2,
	};

	inline uint32_t head_size() const
	{
		return pos_size + sizeof(uint16_t);
	}

	inline uint32_t meta_size() const
	{
		return size_model + size_option;
	}

	inline uint32_t meta_size(IN const MessageMeta& meta) const
	{
		uint32_t pos = size_model + size_option;
		if (eco::has(meta.m_option, option_sess))
			pos += size_session_id;
		if (eco::has(meta.m_option, option_type))
			pos += size_type;
		pos += size_req(meta);
		return pos;
	}

	inline void encode_append_head(
		OUT eco::String& bytes,
		IN  uint32_t head_siz,
		IN  uint8_t  version,
		IN  uint16_t category) const
	{
		bytes.append(head_siz, 0);
		bytes[pos_version] = version;
		bytes[pos_category] = static_cast<uint8_t>(category);
	}

	virtual uint32_t encode_head_size() const
	{
		return head_size();
	}

	virtual uint32_t encode_size(eco::String& bytes) const
	{
		hton(&bytes[pos_size], (uint16_t)(bytes.size() - head_size()));
		return 0;
	}

////////////////////////////////////////////////////////////////////////////////
public:
	TcpProtocol(
		IN eco::net::Check* check = new CheckAdler32(),
		IN eco::net::Crypt* crypt = new CryptFlate())
		: m_crypt(crypt), m_check(check)
	{
		set_version(1);
		set_max_size(0xFFFF);
	}

	virtual uint32_t version_size() const override
	{
		return pos_size;
	}

	virtual bool encode(
		OUT eco::String& bytes,
		OUT uint32_t& start,
		IN  const eco::net::MessageMeta& meta) override
	{
		// 1.init bytes size.
		uint32_t head_siz = encode_head_size();			// encode head size.
		uint32_t byte_siz = meta_size(meta);			// meta size.
		uint32_t code_siz = !meta.m_codec->empty()
			? meta.m_codec->byte_size() : 0;
		byte_siz += code_siz;							// message size.
		if (eco::has(meta.m_category, category_encrypted) && m_crypt.get())
		{
			byte_siz = m_crypt->byte_size(byte_siz);	// crypt size.
		}
		if (eco::has(meta.m_category, category_checksum) && m_check.get())
		{
			byte_siz += m_check->byte_size();			// checksum size.
		}
		if (byte_siz > max_size())
		{
			ECO_THIS_ERROR(e_message_overszie)
				< "message is too large to send="
				< byte_siz < '>' < max_size();
			return false;
		}
		byte_siz += head_siz;
		bytes.clear();
		bytes.reserve(byte_siz);

		// 2.init message version and category.
		encode_append_head(bytes, head_siz, version(), meta.m_category);

		// 3.init message type and optional data.
		bytes.append(meta.m_model);
		bytes.append(meta.m_option);
		if (eco::has(meta.m_option, option_sess))
			append_hton(bytes, meta.m_session_id);
		if (eco::has(meta.m_option, option_type))
			append_hton(bytes, static_cast<uint16_t>(meta.m_message_type));
		// 3.1.optional data: request data.
		encode_req(bytes, meta);

		// 5.encode message object.
		if (!meta.m_codec->empty())
			meta.m_codec->encode_append(bytes, code_siz);

		// 6.encrypt message.
		if (eco::has(meta.m_category, category_encrypted) && m_crypt.get())
		{
			bytes = m_crypt->encode(bytes, head_siz);
			if (bytes.null())
			{
				ECO_THIS_ERROR(e_message_encode) < "crypt message fail.";
				return false;
			}
		}

		// 7.append checksum.
		if (eco::has(meta.m_category, category_checksum) && m_check.get())
		{
			m_check->encode(bytes);
		}

		// 8.reset bytes size.
		start = encode_size(bytes);
		return true;
	}

	virtual void encode_heartbeat(OUT eco::String& bytes) const override
	{
		//encode_append(bytes, version(), category_heartbeat);
		encode_append_head(bytes, head_size(), version(), 2);
	}

	/*@get version/category/version_size.*/
	virtual eco::Result decode_version(
		OUT eco::net::MessageHead& head,
		IN const char* bytes,
		IN const uint32_t size) const override
	{
		if (size >= pos_size)
		{
			head.m_version = bytes[pos_version];
			head.m_category = bytes[pos_category];
			return eco::ok;
		}
		return eco::fail;
	}

	virtual bool decode_size(
		OUT IN eco::net::MessageHead& head,
		IN  const char* bytes,
		IN  const uint32_t size) const override
	{
		if (size >= head_size())
		{
			head.m_head_size = head_size();
			if (!is_heartbeat(head.m_category))
			{
				head.m_data_size = ntoh16(bytes + pos_size);
			}
			return true;
		}
		return false;
	}

	virtual bool decode_meta(
		OUT eco::net::MessageMeta& meta,
		OUT eco::Bytes& data,
		IN  eco::String& bytes,
		IN  uint32_t head_size) override
	{
		// check sum message.
		uint32_t check_sum_size = 0;
		if (eco::has(meta.m_category, category_checksum) && m_check.get())
		{
			if (!m_check->decode(bytes))
			{
				ECO_THIS_ERROR(e_message_checksum)
					< "checksum bytes size error.";
				return false;
			}
			check_sum_size = m_check->byte_size();
		}

		// decrypt message.
		if (eco::has(meta.m_category, category_encrypted) && m_crypt.get())
		{
			uint32_t crypt_size = bytes.size();
			crypt_size -= (head_size + check_sum_size);
			bytes = m_crypt->decode(bytes, head_size, crypt_size);
			if (bytes.null())
			{
				return false;
			}
		}

		// get meta fixed: model & option.
		uint32_t option_pos = head_size + meta_size();
		if (bytes.size() < option_pos)
		{
			ECO_THIS_ERROR(e_protocol_parameter)
				< "message size have no meta: "
				< bytes.size() < '<' < option_pos;
			return false;
		}
		meta.m_model = MessageModel(bytes[head_size]);
		meta.m_option = MessageOption(bytes[head_size + size_model]);

		// get meta option data.
		uint32_t data_pos = head_size + meta_size(meta);
		if (bytes.size() < data_pos)
		{
			ECO_THIS_ERROR(e_protocol_parameter)
				< "message size have no meta option: "
				< bytes.size() < '<' < data_pos;
			return false;
		}
		uint32_t pos = option_pos;
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

		// option data: request data.
		pos += decode_req(meta, &bytes[pos]);

		// message data bytes.
		data.m_data = &bytes[pos];
		data.m_size = bytes.size() - pos - check_sum_size;
		return true;
	}

private:
	std::auto_ptr<Crypt> m_crypt;
	std::auto_ptr<Check> m_check;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif