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
#include <eco/ExportApi.h>
#include <eco/net/protocol/TcpProtocol.h>


namespace eco{;
namespace net{;
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
		// 1byte: "<=253"
		// 3byte=1+2: ">253"
		// 5byte=1+4: ">65535"
		size_byte1			= 1,
		size_byte2			= 2,
		size_byte4			= 4,
		size_encode_size	= 5,	// 4 + 1;
	};

	inline uint32_t get_size_size(IN uint8_t size) const
	{
		if (size < 254) return 0;
		else if (size == 254) return 2;
		else if (size == 255) return 4;
		return 0;
	}

	virtual uint32_t encode_head_size() const override
	{
		return pos_size + size_encode_size;
	}

	virtual uint32_t encode_size(eco::String& bytes) const override
	{
		uint32_t pos = size_byte4;	// bytes start pos.
		uint32_t data_size = bytes.size() - encode_head_size();
		if (data_size < 254)
		{
			bytes[pos + pos_size] = uint8_t(data_size);
		}
		else if (data_size <= max_uint16())
		{
			pos -= size_byte2;
			bytes[pos + pos_size] = char(254);
			hton(&bytes[pos + pos_size + size_byte1], uint16_t(data_size));
		}
		else if (data_size <= max_uint32())
		{
			pos -= size_byte4;
			bytes[pos + pos_size] = char(255);
			hton(&bytes[pos + pos_size + size_byte1], data_size);
		}

		// move "head version category" to pos.
		bytes[pos] = bytes[0];
		bytes[pos + 1] = bytes[1];
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
			pos += size_byte1;
			head.m_head_size = pos + get_size_size(head.m_data_size);
			if (size >= head.m_head_size)
			{
				if (head.m_data_size == 254)
					head.m_data_size = ntoh16(&bytes[pos]);
				else if (head.m_data_size == 255)
					head.m_data_size = ntoh32(&bytes[pos]);
				/*else if (head.m_data_size == 0)
					head.m_data_size = ntoh64(&bytes[pos]); */
				return true;
			}
		}
		return false;
	}

private:
	std::auto_ptr<Crypt> m_crypt;
	std::auto_ptr<Check> m_check;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif