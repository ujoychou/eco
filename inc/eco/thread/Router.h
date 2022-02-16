#ifndef ECO_ROUTER_H
#define ECO_ROUTER_H
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-17.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/thread/Worker.h>
#include <unordered_map>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename msg_t>
class RouterHandler
{
	ECO_OBJECT(RouterHandler);
public:
	struct Message
	{
		inline Message() {}

		inline Message(const type_t& type, const msg_t& msg)
			: m_type(type), m_msg(msg) {}

		inline Message(const type_t& type, msg_t&& msg)
			: m_type(type), m_msg(msg) {}

		type_t m_type;
		msg_t  m_msg;
	};
	typedef std::function<void(IN msg_t&)> HandlerFunc;

	inline RouterHandler() {}

	/*@ dispatch message to message handler.
	* @ para.msg: message to be dispatched.
	*/
	inline void operator()(IN Message& wrap) const
	{
		dispatch(wrap.m_type, wrap.m_msg);
	}

	inline bool dispatch(IN const type_t& type, IN msg_t& msg) const
	{
		eco::this_thread::dead_lock().begin();
		// get message type id and dispatch to the handler.
		auto it = m_handler_map.find(type);
		if (it != m_handler_map.end())
		{
			it->second(msg);
			return true;
		}
		else if (m_default_handler)
		{
			m_default_handler(msg);
			return true;
		}
		return false;
	}
	
	/*@ add message and handler map.*/
	void set_dispatch(IN const type_t& type, IN HandlerFunc handler)
	{
		m_handler_map[type] = handler;
	}

	/*@ set message default handler.*/
	void set_default(IN HandlerFunc handler)
	{
		m_default_handler = handler;
	}

private:
	// default message handler to process unregistered message type.
	HandlerFunc m_default_handler;
	// message and message handler map.
	std::unordered_map<type_t, HandlerFunc>	m_handler_map;
};


////////////////////////////////////////////////////////////////////////////////
template<
	typename type_t, typename msg_t,
	typename Message = typename RouterHandler<type_t, msg_t>::Message >
class Router : public eco::Worker<Message, RouterHandler<type_t, msg_t> >
{
public:
	/*@ add message and handler map.*/
	typedef typename RouterHandler<type_t, msg_t>::HandlerFunc HandlerFunc;
	inline void set_dispatch(IN const type_t& type, IN HandlerFunc func)
	{
		this->m_handler.set_dispatch(type, func);
	}

	/*@ set message default handler.*/
	inline void set_default(IN HandlerFunc func)
	{
		this->m_handler.set_default(func);
	}

	inline void dispatch(IN const type_t& type, IN msg_t&& msg)
	{
		post(Message(type, std::move(msg)));
	}

	inline void dispatch(IN const type_t& type, IN const msg_t& msg)
	{
		post(Message(type, msg));
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif