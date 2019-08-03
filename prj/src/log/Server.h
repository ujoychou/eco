#ifndef ECO_LOG_SERVER_H
#define ECO_LOG_SERVER_H
/*******************************************************************************
@ name
log server.

@ function

@ exception

@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/thread/MessageServer.h>
#include "Queue.h"


namespace eco{;
namespace log{;


////////////////////////////////////////////////////////////////////////////////
template<typename HandlerT>
class Server : public eco::detail::MessageServer<eco::Bytes, HandlerT, Queue>
{
	ECO_OBJECT(Server);
public:
	inline Server()
	{}

	/*@ set message queue max sync interval mill seconds.*/
	inline void set_sync_interval(IN const uint32_t millsec)
	{
		m_message_queue.set_sync_interval(millsec);
	}

	/*@ work thread method.	*/
	virtual void work() override
	{
		try 
		{
			Queue::PackPtr pack;
			while (true)
			{
				m_message_queue.pop(pack);
				// message queue is close and has handled all message.
				if (pack == nullptr && m_message_queue.is_close())
				{
					break;
				}
				m_message_handler(*pack);
			}// end while
		}
		catch (std::exception& e)
		{
			fprintf(stderr, "logging server caught exception: %s\n", e.what());
		}
	}
};



////////////////////////////////////////////////////////////////////////////////
}}
#endif