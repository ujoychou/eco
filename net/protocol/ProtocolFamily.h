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


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
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
	inline eco::Result on_decode_head(
		MessageHead& head, const char* buff, uint32_t size) const
	{
		/*@ eco::fail: check the message edge.
		1.if message bytes not enough to check, need read more bytes.
		--A.version bytes not enough.
		--B.size bytes not enough.

		2.eco::error:
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
		auto res = prot->decode_version(head, buff, size);	// (1.A)
		if (res != eco::ok) return res;

		if (!is_heartbeat(head.m_category) &&
			!eco::has(head.m_category, category_message))	// (2.A)
		{
			ECO_THIS_ERROR(e_message_category)
				< "category error: " < head.m_category;
			return eco::error;
		}

		// #.get protocol by head version.
		head.m_protocol = protocol(head.m_version);
		if (head.m_protocol == nullptr)	// (2.B)
		{
			ECO_THIS_ERROR(e_protocol_invalid)
				< "protocol ver error: " < head.m_version;
			return eco::error;
		}
		// message size = head_size + size_size + data_size.
		if (!head.m_protocol->decode_size(head, buff, size))	// (1.B)
		{
			return eco::fail;
		}
		if (head.message_size() > size)							// (1.B)
		{
			return eco::fail;
		}
		if (head.message_size() > head.m_protocol->max_size())	// (2.C)
		{
			ECO_THIS_ERROR(e_message_overszie)
				< "message size over max size: "
				< head.message_size() < '>' < prot->max_size();
			return eco::error;
		}
		return eco::ok;		// (3.A/B)
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif