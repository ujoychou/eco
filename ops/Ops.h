#ifndef ECO_NET_OPS_H
#define ECO_NET_OPS_H
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
#include <eco/rx/RxExport.h>
#include <eco/detail/proto/Monitor.pb.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
ECO_NS_BEGIN(ops);
////////////////////////////////////////////////////////////////////////////////
enum SessionState
{
	sess_state_living	= 0,	// normal living session.
	sess_state_closed	= 1,	// normal closed session.
	sess_state_broken	= 2,	// peer broken because of heartbeat.
	sess_state_attack	= 3,	// attack session, frequently open/close peer.
};

enum ExceptMode
{
	sess_except_dead	= 0,	// unstable network.
	sess_except_ddos	= 1,	// ddos network.
};

////////////////////////////////////////////////////////////////////////////////
enum TopicId
{
	topic_warning		= 1000,	// eco ops warning topic.
};

enum TopicType
{
	ttype_session		= 1000,	// session topic type.
};

enum TopicProp
{
	tprop_session_id	= 1000,	// session id.
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(ops);
ECO_NS_END(net);
ECO_NS_END(eco);
#endif