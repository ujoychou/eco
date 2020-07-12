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
template<typename MessageType, typename Message>
class DispatchHandler
{
	ECO_OBJECT(DispatchHandler);
public:
	struct Wrap
	{
		inline Wrap()
		{}

		inline Wrap(const MessageType& type, const Message& msg)
			: m_type(type), m_msg(msg)
		{}

		inline Wrap(const MessageType& type, Message&& msg)
			: m_type(type), m_msg(msg)
		{}

		Message		m_msg;
		MessageType m_type;
	};
	typedef std::function<void(IN Message&)> HandlerFunc;
	typedef std::unordered_map<MessageType, HandlerFunc> HandlerMap;

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

	inline void dispatch(IN const MessageType& type, IN Message& msg) const
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
			ECO_ERROR << "dispatch unknown message type: " << type;
		}
	}
	
	/*@ add message and handler map.*/
	void set_dispatch(IN const MessageType& type, IN HandlerFunc handler)
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
	typename MessageType, typename Message,
	typename MessageWrap = DispatchHandler<MessageType, Message>::Wrap >
class DispatchServer :
	public eco::MessageServer<MessageWrap, DispatchHandler<MessageType, Message> >
{
public:
	typedef DispatchHandler<MessageType, Message> ThisType;
	typedef typename ThisType::HandlerFunc HandlerFunc;

	/*@ add message and handler map.*/
	inline void set_dispatch(IN const MessageType& type, IN HandlerFunc func)
	{
		message_handler().set_dispatch(type, func);
	}

	/*@ set message default handler.*/
	inline void set_default(IN HandlerFunc func)
	{
		message_handler().set_default(func);
	}

	inline void dispatch(IN const MessageType& type, IN Message&& msg)
	{
		post(MessageWrap(type, msg));
	}

	inline void dispatch(IN const MessageType& type, IN const Message& msg)
	{
		post(MessageWrap(type, msg));
	}
};


////////////////////////////////////////////////////////////////////////////////
}
#endif