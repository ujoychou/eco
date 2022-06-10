#ifndef ECO_NET_TCP_PROTOCOL2_H
#define ECO_NET_TCP_PROTOCOL2_H
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
#include <eco/rx/RxApi.h>
#include <eco/net/protocol/TcpProtocol.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class TcpProtocol2 : public TcpProtocol
{
private:
	// "message data" class.
	struct __Size__
	{
		// message bytes size.
		uint8_t		m_size1;
		uint16_t	m_size2;
		uint16_t	m_size4;
	};

	// "message data" value
	enum
	{
		// 1byte: "<253"
		// 3byte=1+2: ">=253"
		// 5byte=1+4: ">65535"
		size_byte1			= 1,
		size_byte2			= 2,
		size_byte4			= 4,
		size_encode_size	= 5,	// 4 + 1;
	};

	virtual uint32_t encode_head_size() const override
	{
		return pos_size + size_encode_size;
	}

	virtual uint32_t encode_size(eco::String& bytes) const override
	{
		uint32_t data_size = bytes.size() - encode_head_size();
		uint32_t pos = size_encode_size - size_size(data_size);

		// move "head version category" to pos.
		bytes[pos] = bytes[0];
		bytes[pos + 1] = bytes[1];
		size_encode(&bytes[pos + pos_size], data_size);
		return pos;
	}

////////////////////////////////////////////////////////////////////////////////
public:
	TcpProtocol2(
		IN eco::net::Check* check = new CheckAdler32(),
		IN eco::net::Crypt* crypt = new CryptFlate())
		: TcpProtocol(check, crypt)
	{
		set_version(2);
		set_max_size(max_uint32());
	}

	virtual void encode_heartbeat(OUT eco::String& bytes) const override
	{
		bytes.append(pos_size, 0);
		bytes[pos_version] = version();
		bytes[pos_category] = category_heartbeat;
	}

	virtual bool decode_size(
		OUT IN eco::net::MessageHead& head,
		IN  const char* bytes,
		IN  const uint32_t size) const override
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

private:
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

	std::unique_ptr<Crypt> m_crypt;
	std::unique_ptr<Check> m_check;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif