#ifndef ECO_NET_PROTOCOL_VERSION_H
#define ECO_NET_PROTOCOL_VERSION_H
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
#include <eco/rx/RxHeap.h>
#include <eco/Object.h>
#include <eco/net/Net.h>
#include <eco/net/Ecode.h>


ECO_NS_BEGIN(eco);
namespace net{;
class Protocol;
////////////////////////////////////////////////////////////////////////////////
// support message option for message to send.
enum
{
	// category: heartbeat message.
	category_heartbeat			= 0x00,
	// category: context message.
	category_message			= 0x01,
	// category: message with a check sum.
	category_checksum			= 0x08,
	// category: encrypted message.
	category_encrypted			= 0x10,
	// category: sync request, else aysnc.
	category_sync				= 0x20,
	// category: session mode.
	category_session			= 0x40,
};
// for user define: MessageCategory is uint16_t.
typedef uint16_t MessageCategory;
inline bool is_heartbeat(MessageCategory category)
{
	return (category == category_heartbeat) || eco::has(category, 2);
}


////////////////////////////////////////////////////////////////////////////////
class MessageHead
{
public:
	uint16_t		m_version;
	MessageCategory m_category;
	uint32_t		m_head_size;
	uint32_t		m_data_size;
	Protocol*		m_protocol;

	inline MessageHead()
	{
		memset(this, 0, sizeof(*this));
	}

	inline uint32_t message_size() const
	{
		return m_head_size + m_data_size;
	}
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API ProtocolVersion : public eco::RxHeap
{
public:
	inline static uint16_t max_uint16()
	{
		return 0xFFFF;		// 65535;
	}
	inline static uint32_t max_uint32()
	{
		return 0xFFFFffff;	// 4294967295;
	}

	/*@ get message head size. check same protocol cluster.*/
	virtual uint32_t version_size() const = 0;

	/*@ decode message head.
	* @ para.bytes: message head bytes.
	*/
	virtual eco::Result decode_version(
		OUT eco::net::MessageHead& head,
		IN const char* bytes,
		IN const uint32_t size) const = 0;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif