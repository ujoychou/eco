#ifndef ECO_NET_WEBSOCKET_PROTOCOL
#define ECO_NET_WEBSOCKET_PROTOCOL
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-17.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2017 - 2020, ujoy, reserved all right.

*******************************************************************************/
#include <eco/codec/base64.h>
#include <eco/codec/sha1.h>
#include <eco/net/protocol/Protocol.h>
#include <eco/net/Net.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
const char eco_masking_key[] = "#2^1";
////////////////////////////////////////////////////////////////////////////////
class WebSocket : public Protocol
{
public:
	enum
	{
		websocket_frame_cont		= 0x0,
		websocket_frame_text		= 0x1,
		websocket_frame_binary		= 0x2,
		// 0x3 - 0x7 reserved for non control frame.
		websocket_frame_close		= 0x8,
		websocket_frame_ping		= 0x9,
		websocket_frame_pong		= 0xA,
		// 0xB - 0xF reserved for control frame.
	};
	typedef uint8_t WebSocketFrame;

	// "message data" value
	enum
	{
		// #.websocket.
		size_ws_head		= 2,
		size_ws_len1		= 1,
		size_ws_len2		= 2,
		size_ws_len8		= 8,
		size_ws_mask_key	= 4,
	};

	// "protocol head data" class.
	struct __Head__
	{
		// 1bit-fin,3bit-optcode(type).
		uint8_t		m_fin_optcode;

		// 1bit-mask,3bit-payload len.
		uint8_t		m_mask_payload_len;

		// extend payload len when payload len = 126.
		uint16_t	m_extend_payload_len;

		// mask data 4 bytes if has mask.
		char		m_mask_key[4];
	};
	
protected:
	inline uint32_t pre_size_ws(IN bool mask) const
	{
		uint32_t result = size_ws_head;
		if (mask)
		{
			result += size_ws_mask_key;
		}
		return result += size_ws_len8;	// uint64 or uint16 or 0.	
	}

	inline uint32_t payload_len_size(IN uint8_t payload_len) const
	{
		if (payload_len < 126) return 0;
		else if (payload_len == 126) return 2;
		else if (payload_len == 127) return 8;
		return 0;
	}

	inline void pre_encode_append(OUT eco::String& bytes, IN bool mask) const
	{
		bytes.append(size_ws_head, 0);
		// fin=1,opcode=2;
		bytes[0] = websocket_frame_binary;
		bytes[0] = (uint8_t(bytes[0]) | 0x80);
		// mask=0, payload len dynamic;
		bytes.append(size_ws_len8, 0);
		if (mask)
		{
			bytes.append(size_ws_mask_key, 0);
		}
	}

	inline void mask_data(
		OUT eco::String&  bytes,
		IN const uint32_t start,
		IN const char* masking_key) const
	{
		char*  data = &bytes[start];
		size_t size = bytes.size() - start;
		for (size_t i = 0; i < size; ++i)
		{
			data[i] = data[i] ^ masking_key[i % size_ws_mask_key];
		}
	}

	// return bytes start.
	inline uint32_t encode_mask(OUT eco::String& bytes, IN bool mask) const
	{
		// payload len.
		uint32_t pos = size_ws_len8;	// bytes start pos.
		uint32_t data_size = bytes.size() - pre_size_ws(mask);
		if (data_size < 126)
		{
			bytes[pos + 1] = uint8_t(data_size);
		}
		else if (data_size <= max_uint16())
		{
			pos -= size_ws_len2;
			bytes[pos + 1] = 126;
			hton(&bytes[pos + size_ws_head], (uint16_t)data_size);
		}
		else //if (data_size > max_uint16())
		{
			pos -= size_ws_len8;
			bytes[pos + 1] = 127;
			hton(&bytes[pos + size_ws_head], (uint64_t)data_size);
		}
		bytes[pos] = bytes[0];
		
		// masking key.
		if (mask)
		{
			bytes[pos + 1] |= char(0x80);
			uint32_t end = size_ws_head + size_ws_len8;
			eco::copy(&bytes[end], end, eco_masking_key, size_ws_mask_key);
			mask_data(bytes, end, eco_masking_key);
		}
		return pos;
	}

public:
	inline bool encode_websocket_text(
		OUT eco::String& bytes,
		OUT uint32_t& start,
		IN  const char* message,
		IN  const bool mask)
	{
		// 1.init bytes size.
		uint32_t code_siz = (uint32_t)strlen(message);
		uint32_t byte_siz = pre_size_ws(mask);
		byte_siz += code_siz;
		bytes.clear();
		bytes.reserve(byte_siz);

		// 2.init message version and category.
		pre_encode_append(bytes, mask);
		// 4.encode message object.
		bytes.append(message, code_siz);
		// 5.reset bytes size.
		start = encode_mask(bytes, mask);
		return true;
	}

	/*@decode websocket head detail.
	@return payload_len_end>0: size enough; 
	@return payload_len_end==0: size not enough. 
	*/
	inline uint32_t decode_websocket(
		OUT uint8_t& fin,
		OUT uint8_t& frame,
		OUT uint64_t& payload_len,
		OUT uint8_t& mask,
		IN  const char* byte,
		IN  const uint32_t size) const
	{
		uint32_t pos = 0;
		fin = (uint8_t(byte[pos]) >> 7) > 0;	// is last package.
		frame = (uint8_t(byte[pos]) & 0x0f);	// package type.
		++pos;
		mask = (uint8_t(byte[pos]) >> 7) > 0;	// whether has a mask.
		payload_len = uint8_t(byte[pos]) & 0x7F;
		++pos;

		// get extend payload len.
		uint16_t len_end = pos + payload_len_size((uint8_t)payload_len);
		if (len_end <= size)
		{
			if (payload_len == 126)
				payload_len = eco::net::ntoh16(&byte[pos]);
			else if (payload_len == 127)
				payload_len = eco::net::ntoh64(&byte[pos]);
			return len_end;
		}
		return 0;
	}

	/*@logging websocket detail.*/
	inline void log_websocket(
		char fin, char frame, uint64_t payload_len,
		char mask, const char* payload) const
	{
		ECO_INFO << "[WebSocket] fin=" << int(fin)
			<< " opcode=" << int(frame) 
			<< " mask=" << bool(mask > 0)
			<< " payload_len=" << payload_len
			<< " payload=" << payload;
	}

	/*@decode websocket head detail: payload_len_end, mask, message_size.
	@return payload_len_end >0: size enough;
	@return payload_len_end==0: size not enough.
	*/
	inline uint32_t decode_websocket(
		OUT uint8_t& mask,
		OUT uint32_t& message_size,
		IN  const char* byte,
		IN  const uint32_t size,
		IN  bool logging = false) const
	{
		uint8_t  fin = 0;
		uint8_t  frame = 0;
		uint64_t payload_len = 0;
		uint32_t len_end = decode_websocket(
			fin, frame, payload_len, mask, byte, size);
		if (len_end > 0)	// pos = ws_size + payload_size.
		{
			uint64_t size = payload_len + len_end;
			if (mask) size += 4;

			if (frame == websocket_frame_close)
			{
				ECO_THROW(e_peer_client_close)
					<< "websocket client send close frame.";
			}
			if ((size >> 32) > 0)
			{
				ECO_THROW(e_message_overszie)
					<< "websocket decode payload len > uint32': " < size;
			}
			message_size = uint32_t(size);

			if (logging)	// logging websocket detail to debug.
			{
				log_websocket(fin, frame, payload_len, mask, &byte[len_end]);
			}
		}
		return len_end;
	}

	inline uint32_t decode_websocket(IN eco::String& byte) const
	{
		uint8_t mask = 0;
		uint32_t size = 0;
		uint32_t pos = decode_websocket(mask, size, byte.c_str(), byte.size());
		if (mask && pos > 0)
		{
			char masking_key[4] = { 0 };
			eco::copy(masking_key, pos, &byte[pos], size_ws_mask_key);
			mask_data(byte, pos, masking_key);
		}
		return pos;
	}
};


////////////////////////////////////////////////////////////////////////////////
class WebSocketProtocol : public WebSocket
{
private:
	// "protocol head data" class.
	struct __Head__
	{
		WebSocket::__Head__ m_head;

		// defined: message version.
		uint8_t		m_version;

		// defined: message category.
		uint8_t		m_category;
	};

	// "message data" class.
	struct __Data__
	{
		// message model.
		uint8_t		m_model;
		
		// message option.
		uint8_t		m_option;

		// message type.
		uint16_t	m_message_type;
		uint64_t	m_request_data;
	};

	// "message data" value
	enum
	{
		// #.version.
		size_version		= 1,
		size_category		= 1,

		// #.parameter head
		size_model			= 1,
		size_option			= 1,
		size_type			= 2,
	};

	inline uint32_t ver_cat_size() const
	{
		return size_version + size_category;
	}

	inline uint32_t pre_size(IN bool mask) const
	{
		uint32_t size = WebSocket::pre_size_ws(mask);
		return size + ver_cat_size();
	}

	inline uint32_t meta_size(
		IN const eco::net::MessageMeta& meta) const
	{
		uint32_t size = size_model + size_option;
		if (eco::has(meta.m_option, option_type))
			size += size_type;
		size += size_req(meta);
		return size;
	}

	inline void pre_encode_append(
		OUT eco::String& bytes,
		IN  const uint8_t version,
		IN  const bool mask,
		IN  const MessageCategory category) const
	{
		WebSocket::pre_encode_append(bytes, mask);
		bytes.append(1, version);
		bytes.append(1, (uint8_t)category);
	}

public:
	WebSocketProtocol(IN bool mask) : m_mask(mask)
	{
		set_version(1);
	}

	virtual uint32_t version_size() const override
	{
		return 0;
	}
	
	virtual bool encode(
		OUT eco::String& bytes,
		OUT uint32_t& start,
		IN  const eco::net::MessageMeta& meta,
		OUT eco::Error& e) override
	{
		bool mask = eco::has(meta.m_category, category_encrypted) || m_mask;
		// 1.init bytes size.
		uint32_t meta_siz = meta_size(meta);
		uint32_t code_siz = meta.m_codec->byte_size();
		uint32_t byte_siz = pre_size(mask);
		byte_siz += meta_siz + code_siz;
		bytes.clear();
		bytes.reserve(byte_siz);

		// 2.init message version and category.
		pre_encode_append(bytes, version(), mask, meta.m_category);

		// 3.init message type and optional data.
		bytes.append(static_cast<char>(meta.m_model));
		bytes.append(static_cast<char>(meta.m_option));
		if (eco::has(meta.m_option, option_type))
			append_hton(bytes, static_cast<uint16_t>(meta.m_message_type));
		// 3.1.optional data: request data.
		encode_req(bytes, meta);

		// 4.encode message object.
		meta.m_codec->encode_append(bytes, code_siz);

		// 5.reset bytes size.
		start = encode_mask(bytes, mask);
		return true;
	}

	virtual void encode_heartbeat(OUT eco::String& bytes) const override
	{
		bytes.append(size_ws_head, 0);
		// fin=1,opcode=2;
		bytes[0] = websocket_frame_binary;
		bytes[0] = (uint8_t(bytes[0]) | 0x80);
		// mask=0,payload len=2
		bytes[1] = 2;
		bytes.append(1, version());
		//bytes.append(1, category_heartbeat);
		bytes.append(1, 2);
	}

	virtual bool decode_version(
		OUT eco::net::MessageHead& head,
		IN const char* byte,
		IN const uint32_t size) const override
	{
		uint8_t  mask = 0;
		uint32_t msg_size = 0;
		// get playload_len end pos.
		uint32_t pos = WebSocket::decode_websocket(mask, msg_size, byte, size);
		if (pos == 0) return false;

		uint32_t ver_end = pos + ver_cat_size();
		if (mask)
		{
			ver_end += size_ws_mask_key;
			if (size < ver_end) return false;
			char masking_key[4] = { 0 };
			eco::copy(masking_key, pos, &byte[pos], size_ws_mask_key);
			head.m_version = (byte[pos] ^ masking_key[0]);
			head.m_category = (byte[pos + 1] ^ masking_key[1]);
		}
		else
		{
			if (size < ver_end) return false;
			head.m_version = byte[pos];
			head.m_category = byte[pos + 1];
		}
		head.m_head_size = ver_end;
		head.m_data_size = msg_size - head.m_head_size;
		return true;
	}

	virtual bool decode_size(
		OUT IN eco::net::MessageHead& head,
		IN  const char* bytes,
		IN  const uint32_t size) const override
	{
		// message_size has decoded by [decode_version].
		return true;
	}

	virtual bool decode_meta(
		OUT eco::net::MessageMeta& meta,
		OUT eco::Bytes& data,
		IN  eco::String& bytes,
		IN  uint32_t head_size,
		IN  eco::Error& e) override
	{	
		decode_websocket(bytes);

		// get model & option.
		uint32_t option_pos = head_size + size_model + size_option;
		if (bytes.size() < option_pos)
		{
			e.id(e_protocol_parameter) < "message size have no meta: "
				< bytes.size() < '<' < option_pos;
			return false;
		}
		meta.m_model = MessageModel(bytes[head_size]);
		meta.m_option = MessageOption(bytes[head_size + size_model]);

		// get option data.
		uint32_t meta_end = head_size + meta_size(meta);
		if (bytes.size() < meta_end)
		{
			e.id(e_protocol_parameter) < "message size have no meta option: "
				< bytes.size() < '<' < meta_end;
			return false;
		}
		uint32_t pos = option_pos;
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
		data.m_size = bytes.size() - pos;
		return true;
	}

private:
	uint32_t m_mask;
};


////////////////////////////////////////////////////////////////////////////////
class WebSocketProtocol2 : public WebSocketProtocol
{
public:
	inline WebSocketProtocol2(IN bool mask) : WebSocketProtocol(mask)
	{
		set_version(2);
	}

	virtual uint8_t size_req(const MessageMeta& meta) const override
	{
		if (eco::has(meta.m_option, option_data))
		{
			return size_size(meta.m_option_data);
		}
		return 0;
	}

	virtual uint8_t decode_req(
		OUT MessageMeta& meta,
		IN  const char* bytes) const override
	{
		if (eco::has(meta.m_option, option_data))
		{
			return size_decode(meta.m_option_data, bytes);
		}
		return 0;
	}

	virtual void encode_req(
		eco::String& bytes, const MessageMeta& meta) const override
	{
		if (eco::has(meta.m_option, option_data))
		{
			size_encode(bytes, meta.m_option_data);
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#endif