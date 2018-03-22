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
#include <eco/Type.h>
#include <eco/Object.h>
#include <eco/net/protocol/ProtocolHead.h>
#include <eco/net/Net.h>




ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);


////////////////////////////////////////////////////////////////////////////////
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


const char eco_masking_key[] = "#2^1";
////////////////////////////////////////////////////////////////////////////////
class WebSocketProtocolHead : public ProtocolHead
{
public:
	// "protocol head data" class.
	struct __Data__
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

	// "message data" value
	enum
	{
		size_ws_head		= 2,
		size_ws_len2		= 2,
		size_ws_len8		= 8,
		size_ws_mask_key	= 4,
	};

	inline WebSocketProtocolHead()
	{}

	inline uint16_t max_uint16() const
	{
		return 0xFFFF;	// 65535;
	}

	virtual uint32_t size() const override
	{
		return size_ws_head;
	}

	virtual uint32_t max_data_size() const override
	{
		return max_uint16();	// 65535;
	}

	virtual bool decode_data_size(
		OUT uint32_t& data_size,
		IN const char* head,
		IN const uint32_t size,
		OUT eco::Error& e) const override
	{
		uint32_t pos = 0;
		uint8_t  fin = 0;
		uint8_t  frame = 0;
		uint8_t  mask = 0;
		uint32_t payload_len = 0;
		if (!decode_head(fin, frame, payload_len, mask, pos, head, size, e))
		{
			return false;
		}

		// if it's a websocket close frame.
		if (frame == websocket_frame_close)
		{
			e.id(e_peer_client_close) << "web client close this peer.";
			return false;
		}

		// handle mask & payload.
		data_size = payload_len;
		if (mask)
		{
			data_size += 4;
		}
		// head size = 4 include "two payload len."
		if (payload_len < 126)
		{
			data_size -= 2;	
		}
		else if (payload_len >= 126)
		{
			data_size += 0;		// =2-2
		}
		else if (payload_len > max_uint16())
		{
			data_size += 6;		// =8-2
		}
		return true;
	}

	virtual bool decode(
		OUT eco::net::MessageHead& head,
		IN  const eco::String& bytes,
		IN  eco::Error& e) const override
	{
		uint8_t  fin = 0;
		uint8_t  frame = 0;
		uint8_t  mask = 0;
		uint32_t payload_len = 0;
		uint32_t pos = 0;
		if (!decode_head(fin, frame, payload_len, mask, pos,
			bytes.c_str(), bytes.size(), e))
		{
			return false;
		}
		if (mask)
		{
			char masking_key[4] = { 0 };
			eco::cpy_pos(masking_key, pos, &bytes[pos], size_ws_mask_key);
			head.m_version  = (bytes[pos] ^ masking_key[0]);
			head.m_category = (bytes[pos + 1] ^ masking_key[1]);
		}
		else
		{
			head.m_version = bytes[pos];
			head.m_category = bytes[pos + 1];
		}
		return true;
	}

	virtual void encode_heartbeat(
		OUT eco::String& bytes) const override
	{
		bytes.append(size_ws_head, 0);
		bytes[1] = 2;	// payload len = 2
		bytes.append(1, -1);
		bytes.append(1, category_heartbeat);
	}

public:
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

	inline uint32_t pre_size(IN const MessageCategory category) const
	{
		uint32_t result = size();
		if (eco::has(category, category_encrypted))
		{
			result += size_ws_mask_key;
		}
		return result += size_ws_len8;	// uint64 or uint16 or 0.	
	}

	inline uint32_t ws_head_size(IN const MessageCategory category) const
	{
		uint32_t result = size_ws_head;
		if (eco::has(category, category_encrypted))
		{
			result += size_ws_mask_key;
		}
		return result += size_ws_len8;	// uint64 or uint16 or 0.	
	}

	inline void pre_encode_append(
		OUT eco::String& bytes,
		IN  const MessageCategory category) const
	{
		bytes.append(size_ws_head, 0);
		bytes[0] = websocket_frame_binary;		// optcode
		bytes[0] = (uint8_t(bytes[0]) | 0x80);	// fin
		bytes.append(size_ws_len8, 0);
		if (eco::has(category, category_encrypted))
		{
			bytes.append(size_ws_mask_key, 0);
		}
	}


	// return bytes start.
	inline uint32_t encode_mask(
		OUT eco::String& bytes,
		IN  const MessageCategory category) const
	{
		// payload len.
		uint32_t data_start = 0;
		uint32_t data_size = bytes.size() - ws_head_size(category);
		if (data_size < 126)
		{
			data_start = size_ws_len8;
			bytes[data_start] = bytes[0];
			bytes[data_start + 1] = uint8_t(data_size);
		}
		else if (data_size <= max_uint16())
		{
			data_start = size_ws_len8 - size_ws_len2;
			bytes[data_start] = bytes[0];
			bytes[data_start + 1] = 126;
			eco::net::hton(
				&bytes[data_start + size_ws_len2], (uint16_t)data_size);
		}
		else //if (data_size > max_uint16())
		{
			bytes[data_start + 1] = 127;
			eco::net::hton(
				&bytes[data_start + size_ws_len2], (uint64_t)data_size);
		}
		uint32_t pos = size_ws_head + size_ws_len8;

		// masking key.
		if (eco::has(category, category_encrypted))
		{
			bytes[data_start + 1] = uint8_t(bytes[data_start + 1]) | 0x80;
			eco::cpy_pos(&bytes[pos], pos, eco_masking_key, size_ws_mask_key);
			mask_data(bytes, pos, eco_masking_key);
		}
		return data_start;
	}

public:
	inline uint32_t get_payload_len_size(
		IN const uint8_t payload_len) const
	{
		if (payload_len < 126)
			return 0;
		else if (payload_len == 126)
			return 2;
		else if (payload_len == 127)
			return 8;
		return 0;
	}

	inline bool decode_head(
		OUT uint8_t& fin,
		OUT uint8_t& frame,
		OUT uint32_t& payload_len,
		OUT uint8_t& mask,
		OUT IN uint32_t& pos,
		IN  const char* head,
		IN  const uint32_t size,
		IN  eco::Error& e) const
	{
		fin = (uint8_t(head[pos]) >> 7) > 0;	// is last package.
		frame = (uint8_t(head[pos]) & 0x0f);	// package type.
		++pos;
		mask = (uint8_t(head[pos]) >> 7) > 0;	// whether has a mask.
		payload_len = uint8_t(head[pos]) & 0x7F;
		++pos;

		// get extend payload len.
		uint16_t len_end = get_payload_len_size((uint8_t)payload_len) + pos;
		if (size < len_end)
		{
			e.id(-1) << "decode data size fail, head size too small: " << size 
				<< " payload len:" << payload_len;
			return false;
		}
		if (payload_len == 126)
		{
			payload_len = eco::net::ntoh16(&head[pos]);
		}
		else if (payload_len == 127)
		{
			uint64_t v = eco::net::ntoh64(&head[pos]);
			if ((v >> 32) > 0)
			{
				e.id(-1) 
					<< "decode data size fail, 'payload len > uint32_t': " << v;
				return false;
			}
			payload_len = (uint32_t)v;
		}
		pos = len_end;
		return true;
	}

	inline bool decode(OUT eco::String& bytes,
		OUT uint32_t& head_end, IN  eco::Error& e) const
	{
		uint8_t  fin = 0;
		uint8_t  frame = 0;
		uint8_t  mask = 0;
		uint32_t payload_len = 0;
		head_end = 0;
		if (!decode_head(fin, frame, payload_len, mask, head_end,
			bytes.c_str(), bytes.size(), e))
		{
			return false;
		}
		if (mask)
		{
			char masking_key[4] = { 0 };
			eco::cpy_pos(masking_key, head_end,
				&bytes[head_end], size_ws_mask_key);
			mask_data(bytes, head_end, masking_key);
		}
		return true;
	}
};



////////////////////////////////////////////////////////////////////////////////
class WebSocketProtocolHeadEx : public WebSocketProtocolHead
{
public:
	// "protocol head data" class.
	struct __Data__
	{
		// 1bit-fin,3bit-optcode(type).
		WebSocketProtocolHead::__Data__ ws_ph;

		// message version.
		uint8_t		m_version;

		// message category.
		uint8_t		m_category;
	};

	// "message data" value
	enum
	{
		size_version		= 1,
		size_category		= 1,
	};

public:
	virtual uint32_t size() const override
	{
		return size_ws_head + size_version + size_category;
	}

	inline void pre_encode_append(
		OUT eco::String& bytes,
		IN  const uint8_t version,
		IN  const MessageCategory category) const
	{
		WebSocketProtocolHead::pre_encode_append(bytes, category);
		bytes.append(1, version);
		bytes.append(1, (uint8_t)category);
	}

	inline bool decode(OUT eco::String& bytes,
		OUT uint32_t& head_end, IN  eco::Error& e) const
	{
		if (WebSocketProtocolHead::decode(bytes, head_end, e))
		{
			head_end += size_version;
			head_end += size_category;
			return true;
		}
		return false;
	}
};


////////////////////////////////////////////////////////////////////////////////
class WebSocketProtocol : public Protocol
{
private:
	// "message data" class.
	struct __Data__
	{
		// message model.
		uint8_t		m_model;
		
		// message option.
		uint8_t		m_option;

		// message type.
		uint32_t	m_message_type;
		uint64_t	m_request_data;
	};

	// "message data" value
	enum
	{
		// #.parameter head
		size_model			= 1,
		size_option			= 1,
		size_type			= 2,
	};

public:
	WebSocketProtocol(IN const bool mask) : m_mask(mask)
	{}

	inline uint32_t get_meta_size(
		IN const eco::net::MessageMeta& meta) const
	{
		uint32_t size = size_model + size_option;
		if (eco::has(meta.m_option, option_type))
		{
			size += size_type;
		}
		if (eco::has(meta.m_option, option_req4))
		{
			size += sizeof(uint32_t);
		}
		else if (eco::has(meta.m_option, option_req8))
		{
			size += sizeof(uint64_t);
		}
		return size;
	}

	inline const char* decode_websocket(IN  eco::String& bytes)
	{
		uint32_t pos = 0;
		uint8_t  fin = 0;
		uint8_t  frame = 0;
		uint8_t  mask = 0;
		uint32_t payload_len = 0;
		eco::Error e;
		WebSocketProtocolHead head;
		if (!head.decode_head(fin, frame, payload_len, mask,
			pos, bytes.c_str(), bytes.size(), e))
		{
			EcoError << e;
			return nullptr;
		}
		if (mask)
		{
			char masking_key[4] = { 0 };
			eco::cpy_pos(masking_key, pos, &bytes[pos],
				WebSocketProtocolHead::size_ws_mask_key);
			head.mask_data(bytes, pos, masking_key);
		}
		
		EcoInfo << "[WebSocket] " 
			<< " fin=" << int(fin)
			<< " opcode=" << int(frame)
			<< " mask=" << bool(mask != 0)
			<< " payload len=" << payload_len
			<< " payload=" << &bytes[pos];
		return &bytes[pos];
	}

public:
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
		WebSocketProtocolHeadEx head;
		uint32_t meta_pos = 0;
		if (!head.decode(bytes, meta_pos, e))
		{
			return false;
		}

		// get model & option.
		uint32_t meta_opt_pos = meta_pos + size_model + size_option;
		if (bytes.size() < meta_opt_pos)
		{
			e.id(e_protocol_parameter)
				<< "message size is too smaller to have option: "
				<< bytes.size() << '<' << meta_opt_pos;
			return false;
		}
		meta.m_model = MessageModel(bytes[meta_pos]);
		meta.m_option = MessageOption(bytes[meta_pos + size_model]);

		// get option data.
		uint32_t meta_end = meta_pos + get_meta_size(meta);
		if (bytes.size() < meta_end)
		{
			e.id(e_protocol_parameter)
				<< "message size is too small to have meta: "
				<< bytes.size() << '<' << meta_end;
			return false;
		}
		uint32_t pos = meta_opt_pos;
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
		data.m_size = bytes.size() - pos;
		return true;
	}

	virtual bool encode(
		OUT eco::String& bytes,
		OUT uint32_t& start,
		IN  const eco::net::MessageMeta& meta,
		OUT eco::Error& e) override
	{
		MessageCategory category = meta.m_category;
		eco::set(category, category_encrypted, m_mask > 0);

		WebSocketProtocolHeadEx prot_head;
		// 1.init bytes size.
		uint32_t meta_size = get_meta_size(meta);
		uint32_t code_size = meta.m_codec->get_byte_size();
		uint32_t byte_size = prot_head.pre_size(category);
		byte_size += meta_size + code_size;
		bytes.clear();
		bytes.reserve(byte_size);

		// 2.init message version and category.
		prot_head.pre_encode_append(bytes, version(), category);

		// 3.init message type and optional data.
		bytes.append(static_cast<char>(meta.m_model));
		bytes.append(static_cast<char>(meta.m_option));
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

		// 4.encode message object.
		meta.m_codec->encode_append(bytes, code_size);

		// 5.reset bytes size.
		start = prot_head.encode_mask(bytes, category);
		return true;
	}

	inline bool encode_text(
		OUT eco::String& bytes,
		OUT uint32_t& start,
		IN  const char* message,
		IN  MessageCategory category = category_encrypted)
	{
		WebSocketProtocolHead prot_head;
		eco::set(category, category_encrypted, m_mask > 0);

		// 1.init bytes size.
		uint32_t code_size = (uint32_t)strlen(message);
		uint32_t byte_size = prot_head.pre_size(category);
		byte_size += code_size;
		bytes.clear();
		bytes.reserve(byte_size);

		// 2.init message version and category.
		prot_head.pre_encode_append(bytes, category);

		// 4.encode message object.
		bytes.append(message, code_size);

		// 5.reset bytes size.
		start = prot_head.encode_mask(bytes, category);
		return true;
	}

private:
	uint32_t m_mask;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#endif
