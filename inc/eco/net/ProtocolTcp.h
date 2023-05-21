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
#include <eco/Number.h>
#include <eco/rx/RxHeap.h>
#include <eco/net/Net.h>
#include <eco/net/Ecode.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
class Protocol;
class TcpSocket;
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
};
// for user define: MessageCategory is uint16_t.
typedef uint16_t MessageCategory;
inline bool is_heartbeat(MessageCategory category)
{
	return (category == category_heartbeat) || eco::has(category, 2);
}


////////////////////////////////////////////////////////////////////////////////
class MessageTcp
{
public:
	// message head
	uint16_t		version;
	MessageCategory category;

	// tcp data
	uint32_t		headsize;
	uint32_t		datasize;
	eco::String 	buffer;

	Protocol*		protocol;
	TcpSocket::ref  socket;
	
	inline MessageTcp()
	{
		memset(this, 0, sizeof(*this));
	}

	inline uint32_t size() const
	{
		return headsize + datasize;
	}
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API ProtocolTcp : public eco::RxHeap
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
	virtual uint32_t size() const = 0
	{
		return pos_size;
	}

	/*@ decode message head.
	* @ para.bytes: message head bytes.
	*/
	virtual eco::Result decode(
		OUT eco::net::MessageTcp& tcp,
		IN const char* bytes,
		IN const uint32_t size) const = 0
	{
		if (size >= pos_size)
		{
			head.m_version = bytes[pos_version];
			head.m_category = bytes[pos_category];
			return eco::ok;
		}
		return eco::fail;
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net)
ECO_NS_END(eco)