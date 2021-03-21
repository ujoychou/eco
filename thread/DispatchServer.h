#ifndef ECO_DISPATCH_SERVER_H
#define ECO_DISPATCH_SERVER_H
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
#include <eco/thread/MessageServer.h>
#include <unordered_map>


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
template<typename type_t, typename msg_t>
class DispatchHandler
{
	ECO_OBJECT(DispatchHandler);
public:
	struct Wrap
	{
		inline Wrap()
		{}

		inline Wrap(const type_t& type, const msg_t& msg)
			: m_type(type), m_msg(msg)
		{}

		inline Wrap(const type_t& type, msg_t&& msg)
			: m_type(type), m_msg(msg)
		{}

		msg_t		m_msg;
		type_t m_type;
	};
	typedef std::function<void(IN msg_t&)> HandlerFunc;
	typedef std::unordered_map<type_t, HandlerFunc> HandlerMap;

	// message and message handler map.
	HandlerMap	m_handler_map;

	// default message handler to process unregistered message type.
	HandlerFunc m_default_handler;

public:
	DispatchHandler() : m_default_handler(nullptr)
	{}

	/*@ dispatch message to message handler.
	* @ para.msg: message to be dispatched.
	*/
	inline void operator()(IN Wrap& wrap) const
	{
		dispatch(wrap.m_type, wrap.m_msg);
	}

	inline bool dispatch(IN const type_t& type, IN msg_t& msg) const
	{
		eco::this_thread::lock().set_object(type);

		// get message type id and dispatch to the handler.
		auto it = m_handler_map.find(type);
		if (it != m_handler_map.end())
		{
			it->second(msg);
		}
		else if (m_default_handler)
		{
			m_default_handler(msg);
		}
		else
		{
			return false;
		}
		return true;
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
};


////////////////////////////////////////////////////////////////////////////////
template<
	typename type_t, typename msg_t,
	typename MessageWrap = DispatchHandler<type_t, msg_t>::Wrap >
class DispatchServer :
	public eco::MessageServer<MessageWrap, DispatchHandler<type_t, msg_t> >
{
public:
	typedef DispatchHandler<type_t, msg_t> ThisType;
	typedef typename ThisType::HandlerFunc HandlerFunc;

	/*@ add message and handler map.*/
	inline void set_dispatch(IN const type_t& type, IN HandlerFunc func)
	{
		message_handler().set_dispatch(type, func);
	}

	/*@ set message default handler.*/
	inline void set_default(IN HandlerFunc func)
	{
		message_handler().set_default(func);
	}

	inline void dispatch(IN const type_t& type, IN msg_t&& msg)
	{
		post(MessageWrap(type, msg));
	}

	inline void dispatch(IN const type_t& type, IN const msg_t& msg)
	{
		post(MessageWrap(type, msg));
	}
};


////////////////////////////////////////////////////////////////////////////////
}
#endif