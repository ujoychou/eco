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
#include <eco/net/Protocol.h>
#include <eco/net/codec/Crypt.h>
#include <eco/net/codec/Check.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class ProtocolEco25 : public Protocol
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

		// message type.
		uint16_t	m_type;

		// optional field: the request data from client.
		uint64_t	m_request_data;
	};

	// "message data" value
	enum
	{
		// #.protocol ver.
		pos_version			= 0,
		pos_category		= 1,
		pos_size			= 2,
		
		// 1byte: "<253"
		// 3byte=1+2: ">=253"
		// 5byte=1+4: ">65535"
		size_byte1			= 1,
		size_byte2			= 2,
		size_byte4			= 4,
		size_encode_size	= 5,

		// #.parameter head
		size_model			= 1,
		size_option			= 1,
		// #.parameter option data.
		size_type			= 2,
	};

public:
	inline uint32_t head_size() const
	{
		return pos_size + size_encode_size;
	}

	inline uint32_t meta_size() const
	{
		return size_model + size_option;
	}

	inline uint32_t meta_size(const MessageMeta& meta) const
	{
		uint32_t pos = size_model + size_option;
		if (eco::has(meta.m_option, option_type))
		{
			pos += size_type;
		}
		if (eco::has(meta.m_option, option_data))
		{
			pos += size_size(meta.m_option_data);
		}
		return pos;
	}

	inline void encode_append_head(
		eco::String& bytes,
		uint32_t head_siz,
		uint8_t  version,
		uint16_t category) const
	{
		bytes.append(head_siz, 0);
		bytes[pos_version] = version;
		bytes[pos_category] = static_cast<uint8_t>(category);
	}

	inline uint32_t encode_size(eco::String& bytes) const
	{
		uint32_t data_size = bytes.size() - head_size();
		uint32_t pos = size_encode_size - size_size(data_size);

		// move "head version category" to pos.
		bytes[pos] = bytes[0];
		bytes[pos + 1] = bytes[1];
		size_encode(&bytes[pos + pos_size], data_size);
		return pos;
	}


////////////////////////////////////////////////////////////////////////////////
public:
	inline ProtocolEco25(eco::net::Check* check, eco::net::Crypt* crypt)
	{
		this->version = 2;
		this->maxsize = max_uint32();
		if (m_check == 0) { m_check = new CryptFlate(); }
		if (m_crypt == 0) { m_crypt = new CheckAdler32(); }
	}

	virtual void encode_heartbeat(eco::String& bytes) const override
	{
		bytes.append(pos_size, 0);
		bytes[pos_version] = version();
		bytes[pos_category] = category_heartbeat;
	}

	virtual bool encode(
		eco::String& bytes, uint32_t& start,
		const eco::net::MessageMeta& meta) override
	{
		// 1.init bytes size.
		uint32_t head_siz = head_size();			// encode head size.
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
		if (eco::has(meta.m_option, option_data))
		{
			size_encode(bytes, meta.m_option_data);
		}

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

public:
	virtual bool decode_tcp(
		eco::net::MessageTcp& head,
		const char* bytes, uint32_t size) const override
	{
		if (is_heartbeat(head.m_category))
		{
			head.m_head_size = pos_size;
			return true;
		}

		// get dynamic data size, reference websocket length solution.
		uint32_t pos = pos_size;
		if (size >= pos + size_byte1)
		{
			head.m_data_size = uint8_t(bytes[pos]);	// note: cast to uint8_t.
			head.m_head_size = pos_size + size_size_byte(head.m_data_size);
			if (size >= head.m_head_size)
			{
				size_decode(head.m_data_size, &bytes[pos]);
				return true;
			}
		}
		return false;
	}

	virtual bool decode_meta(
		eco::net::MessageMeta& meta,
		eco::Bytes& data,
		eco::String& bytes,
		uint32_t head_size) override
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
		if (eco::has(meta.m_option, option_data))
		{
			pos += size_decode(meta.m_option_data, bytes);
		}

		// message data bytes.
		data.m_data = &bytes[pos];
		data.m_size = bytes.size() - pos - check_sum_size;
		return true;
	}

private:
	std::unique_ptr<Crypt> m_crypt;
	std::unique_ptr<Check> m_check;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net)
ECO_NS_END(eco)