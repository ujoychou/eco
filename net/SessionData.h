#ifndef ECO_NET_SESSION_DATA_H
#define ECO_NET_SESSION_DATA_H
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
#include <eco/Type.h>
#include <eco/HeapOperators.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
typedef uint32_t SessionId;
const SessionId none_session = 0;

// define session data holder. (using the boost::any<type> mechanism)
class SessionData : public eco::HeapOperators
{
	ECO_OBJECT(SessionData);
public:
	inline  SessionData() {}
	virtual ~SessionData() {}
};

// default session factory function.
template<typename SessionDataT>
inline static SessionData* make_session_data(IN const SessionId& session_id)
{
	return new SessionDataT();
}

// set session factory to create session of tcp server peer.
typedef SessionData* (*MakeSessionDataFunc)(IN const SessionId& session_id);


////////////////////////////////////////////////////////////////////////////////
}}
#endif