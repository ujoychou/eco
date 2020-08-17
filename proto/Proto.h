#ifndef ECO_PROTO_H
#define ECO_PROTO_H
#ifndef ECO_NO_PROTOBUF
/*******************************************************************************
@ 名称

@ 功能

@ 异常

@ 备注

--------------------------------------------------------------------------------
@ 历史记录 @
@ ujoy modifyed on 2016-11-02

--------------------------------------------------------------------------------
* 版权所有(c) 2016 - 2019, siberia corp, 保留所有权利。

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