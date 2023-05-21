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
#include <eco/Error.h>
#include <eco/net/Protocol.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class ECO_API Family
{
	ECO_OBJECT_API(Family);
public:
	// add tcp protocol
	void protocol_tcp(IN ProtocolTcp*);

	// get protocol of lastest version
	ProtocolTcp* protocol_tcp();

	// register protocol
	void protocol_add(IN Protocol*);

	// get protocol by it's version.
	Protocol* protocol(int version);

	// get protocol of lastest version.
	Protocol* protocol_latest();

public:
	eco::Result on_decode_head(MessageTcp& tcp, const char* buff, uint32_t size)
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
		auto res = protocol_tcp()->decode(tcp, buff, size);	// (1.A)
		if (res != eco::ok) { return res; }

		if (!is_heartbeat(tcp.m_category) &&
			!eco::has(tcp.m_category, category_message))	// (2.A)
		{
			ECO_THIS_ERROR(e_message_category)
				< "category error: " < tcp.m_category;
			return eco::error;
		}

		// #.get protocol by head version.
		tcp.m_protocol = protocol(tcp.m_version);
		if (tcp.m_protocol == nullptr)	// (2.B)
		{
			ECO_THIS_ERROR(e_protocol_invalid)
				< "protocol ver error: " < tcp.m_version;
			return eco::error;
		}
		// message size = head_size + size_size + data_size.
		if (!tcp.m_protocol->decode_tcp(head, buff, size))	// (1.B)
		{
			return eco::fail;
		}
		if (tcp.message_size() > size)							// (1.B)
		{
			return eco::fail;
		}
		if (tcp.message_size() > tcp.m_protocol->max_size())	// (2.C)
		{
			ECO_THIS_ERROR(e_message_overszie)
				< "message size over max size: "
				< tcp.message_size() < '>' < prot->max_size();
			return eco::error;
		}
		return eco::ok;		// (3.A/B)
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
ECO_NS_END(net);