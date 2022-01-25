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
#include <eco/thread/Worker.h>
#include "Queue.h"


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(log);
////////////////////////////////////////////////////////////////////////////////
template<typename HandlerT>
class Server : public eco::Worker<eco::Bytes, HandlerT, eco::Thread, Queue>
{
	ECO_OBJECT(Server);
public:
	inline Server()
	{}

	/*@ set message queue max sync interval mill seconds.*/
	inline void set_sync_interval(IN const uint32_t millsec)
	{
		this->m_channel.set_sync_interval(millsec);
	}

	/*@ work thread method.	*/
	virtual void work() override
	{
		try 
		{
			Queue::PackPtr pack;
			while (true)
			{
				this->m_channel.pop(pack);
				// message queue is close and has handled all message.
				if (pack == nullptr && this->m_channel.is_close())
				{
					break;
				}
				this->m_handler(*pack);
			}// end while
		}
		catch (std::exception& e)
		{
			fprintf(stderr, "logging server caught exception: %s\n", e.what());
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(log);
ECO_NS_END(eco);
#endif