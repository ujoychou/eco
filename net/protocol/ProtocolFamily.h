#ifndef ECO_NET_PROTOCOL_FAMILY_H
#define ECO_NET_PROTOCOL_FAMILY_H
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
#include <eco/net/protocol/Protocol.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class ECO_API ProtocolFamily
{
	ECO_OBJECT_API(ProtocolFamily);
public:
	/*@ register protocol.*/
	void add_protocol(IN Protocol*);

	/*@ get protocol by it's version.*/
	Protocol* protocol(int version) const;

	/*@ get protocol of lastest version.*/
	Protocol* protocol_latest() const;

public:
	inline bool on_decode_head(
		MessageHead& head, const char* buff, uint32_t size) const
	{
		/*@ check the message edge.
		1.if message bytes not enough to check, need read more bytes.
		--A.version bytes not enough.
		--B.size bytes not enough.

		2.return false:
		if message is error message:
		--A.category invalid.
		--B.get protocol invalid by version.
		--C.message is max than max size.

		3.if message check edge success, and get message_size.
		--A.heartbeat message.
		--B.general message.
		*/

		// #.get message version & category.
		Protocol* prot = protocol_latest();
		if (!prot->decode_version(head, buff, size))		// (1.A)
		{
			return false;
		}
		if (!is_heartbeat(head.m_category) &&
			!eco::has(head.m_category, category_message))	// (2.A)
		{
			ECO_THROW(e_message_category) << "category error: "
				<< head.m_category;
		}

		// #.get protocol by head version.
		head.m_protocol = protocol(head.m_version);
		if (head.m_protocol == nullptr)	// (2.B)
		{
			ECO_THROW(e_protocol_invalid) << "protocol ver error: "
				<< head.m_version;
		}
		// message size = head_size + size_size + data_size.
		if (!head.m_protocol->decode_size(head, buff, size))	// (1.B)
		{
			return false;
		}
		if (head.message_size() > head.m_protocol->max_size())	// (2.C)
		{
			ECO_THROW(e_message_overszie) << "message size over max size: "
				<< head.message_size() < '>' < prot->max_size();
		}
		return true;		// (3.A/B)
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif