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
#include <eco/Project.h>
#include <eco/thread/MessageServer.h>
#include <unordered_map>


namespace eco{;


////////////////////////////////////////////////////////////////////////////////
template<typename MessageType, typename Message>
class DispatchHandler
{
	ECO_OBJECT(DispatchHandler);
public:
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
	inline void operator()(IN Message& msg) const
	{
		dispatch((MessageType)msg->get_type(), msg);
	}

	inline void dispatch(IN const MessageType& type, IN Message& msg) const
	{
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
			EcoError << "dispatch unknown message type: " << type;
		}
	}
	
	/*@ add message and handler map.*/
	void set_dispatch(IN const MessageType& type, IN HandlerFunc& handler)
	{
		m_handler_map[type] = handler;
	}

	/*@ set message default handler.*/
	void set_default(IN HandlerFunc& handler)
	{
		m_default_handler = handler;
	}
};


////////////////////////////////////////////////////////////////////////////////
template<typename MessageType, typename Message>
class DispatchServer :
	public eco::MessageServer<Message, DispatchHandler<MessageType, Message> >
{
public:
	typedef DispatchHandler<MessageType, Message> ThisType;
	typedef typename ThisType::HandlerFunc HandlerFunc;

	/*@ add message and handler map.*/
	void set_dispatch(IN const MessageType& type, IN HandlerFunc func)
	{
		message_handler().set_dispatch(type, func);
	}

	/*@ set message default handler.*/
	void set_default(IN HandlerFunc func)
	{
		message_handler().set_default(func);
	}
};


////////////////////////////////////////////////////////////////////////////////
}
#endif