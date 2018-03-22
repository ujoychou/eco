#ifndef ECO_NET_ASYNC_HANDLER_MAP_H
#define ECO_NET_ASYNC_HANDLER_MAP_H
/*******************************************************************************
@ name
	thread safe async map.

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/thread/Map.h>
#include <eco/net/RequestHandler.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class AsyncHandlerMap
{
	ECO_OBJECT(AsyncHandlerMap);
public:
	inline AsyncHandlerMap() {};

	// async id management, identify async message.
	inline uint32_t next_request_id()
	{
		uint32_t req_id = ++m_request_id;
		if (req_id == 0)	// note: request != 0.
			req_id = ++m_request_id;
		return req_id;
	}

	// has async message handler.
	inline bool has_async(IN const uint32_t req_id)
	{
		return m_async_map.has(req_id);
	}

	// pop async message handler.(get and delete)
	inline MessageHandler::ptr pop_async(
		IN const uint32_t req_id, 
		IN const bool last = true)
	{
		int eid = 0;
		if (last) return m_async_map.pop(req_id, eid);
		MessageHandler::ptr handler;
		m_async_map.find(handler, req_id);
		return handler;
	}

	// pop async message handler.(get and delete)
	inline MessageHandler::ptr pop_async(
		IN const uint32_t req_id,
		IN const char* name, 
		IN const bool last = true)
	{
		int eid = 0;
		MessageHandler::ptr handler;
		if (last)
			handler = m_async_map.pop(req_id, eid);
		else 
			m_async_map.find(handler, req_id);

		if (handler == nullptr)
			EcoThrow << name << " pop_async message handler fail.";
		return handler;
	}

	// add async message handler.
	inline uint32_t add_async(MessageHandler::ptr& hdl)
	{
		uint32_t req_id = next_request_id();
		m_async_map.set(req_id, hdl);
		return req_id;
	}

private:
	eco::Atomic<uint32_t> m_request_id;
	eco::HashMap<uint32_t, MessageHandler::ptr> m_async_map;
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net)
ECO_NS_END(eco)
#endif