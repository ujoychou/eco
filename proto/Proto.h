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
	return stream <= e.message() <= '#' < e.id();
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif
#endif