#ifndef ECO_NET_PROTOCOL_HEAD_H
#define ECO_NET_PROTOCOL_HEAD_H
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
#include <eco/Error.h>
#include <eco/net/Net.h>
#include <eco/net/Ecode.h>



namespace eco{;
namespace net{;



////////////////////////////////////////////////////////////////////////////////
// support message option for message to send.
enum
{
	// category: context message.
	category_message			= 0x0001,
	// category: heartbeat message.
	category_heartbeat			= 0x0002,

	// option: a session message.
	category_session_mode		= 0x0008,
	// option: encrypted message, else general message.
	category_encrypted			= 0x0010,
	// option: sync request, else aysnc.
	category_sync_mode			= 0x0020,
	// option: is last array data.
	category_last				= 0x0040,
	// option: has message type to identify message.
	category_has_type			= 0x0080,
};
// for user define: MessageCategory is uint32_t.
typedef uint32_t MessageCategory;


////////////////////////////////////////////////////////////////////////////////
class MessageHead
{
public:
	uint32_t m_version;
	MessageCategory m_category;

	inline MessageHead() : m_version(0), m_category(0)
	{}
};


////////////////////////////////////////////////////////////////////////////////
class ProtocolHead
{
public:
	// get message head size. max head "size = 32 = sizeof(s_head_data[])".
	virtual uint32_t head_size() const = 0;

	// get message data size that don't include head size.
	virtual uint32_t decode_data_size(
		IN const char* bytes) const = 0;

	/*@ decode message head.
	* @ para.bytes: message head bytes.
	*/
	virtual bool decode(
		OUT eco::net::MessageHead& head,
		IN  const eco::String& bytes,
		IN  eco::Error& e) const = 0;

	/*@ encode protocol head.*/
	virtual void encode_append(
		OUT eco::String& bytes,
		IN  const eco::net::MessageHead& head) const = 0;

	virtual void encode_data_size(
		OUT eco::String& bytes) const = 0;

	/* encode a heartbeat message, heartbeat encoded by protocol head, not by 
	protocol, so it has no protocol info. and heartbeat don't judge protocol,
	it's responsibility is only judge peer live status.
	*/
	virtual void encode_heartbeat(
		OUT eco::String& bytes) const
	{
		MessageHead head;
		head.m_version = -1;
		eco::add(head.m_category, category_heartbeat);
		encode_append(bytes, head);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif