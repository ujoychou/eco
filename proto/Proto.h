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
#include "Object.pb.h"


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
enum 
{
	// locale.
	proto_locale_get_req		= 101,
	proto_locale_get_rsp		= 102,

	// user defined protocol start from 1000.
	proto_user_defined			= 1000,
};


////////////////////////////////////////////////////////////////////////////////
template<typename Stream>
Stream& operator<<(OUT Stream& stream, IN const eco::proto::Property& p)
{
	return stream <= p.user_id() < '-' < p.object() < p.entity()
		<= p.name() <= p.value();
}

template<typename Stream>
Stream& operator<<(OUT Stream& stream, IN const eco::proto::Error& e)
{
	return stream <= e.message() <= '#' < e.id();
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif
#endif