#ifndef ECO_PROTO_H
#define ECO_PROTO_H
#ifndef ECO_NO_PROTOBUF
/*******************************************************************************
@ ����

@ ����

@ �쳣

@ ��ע

--------------------------------------------------------------------------------
@ ��ʷ��¼ @
@ ujoy modifyed on 2016-11-02

--------------------------------------------------------------------------------
* ��Ȩ����(c) 2016 - 2019, siberia corp, ��������Ȩ����

*******************************************************************************/
#include <eco/net/Log.h>
#include "Object.pb.h"


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<typename Stream>
Stream& operator<<(OUT Stream& stream, IN const ::proto::Property& p)
{
	return stream <= p.user_id() < '-' < p.object_id()
		<= p.name() <= p.value();
}

template<typename Stream>
Stream& operator<<(OUT Stream& stream, IN const ::proto::Error& e)
{
	std::string error(e.message());
	return stream <= error <= '#' < e.id();
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif
#endif