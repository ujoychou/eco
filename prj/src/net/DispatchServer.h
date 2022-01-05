#ifndef ECO_NET_DISPATCH_SERVER_H
#define ECO_NET_DISPATCH_SERVER_H
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
#include <eco/net/Ecode.h>
#include <eco/net/Context.h>
#include <eco/thread/DispatchServer.h>


ECO_NS_BEGIN(eco);
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class DispatchHandler : public eco::DispatchHandler<int, Context>
{
public:
	// set receive mode: recv event.
	inline void set_event(std::function<void(TcpPeer::ptr&, DataContext&)>&& f)
	{
		m_recv = f;
	}

	// is dispatch mode.
	inline bool dispatch_mode() const
	{
		return !m_recv;
	}

	// is receive mode.
	inline bool receive_mode() const
	{
		return (bool)m_recv;
	}

	/*@ dispatch message to message handler.
	* @ para.dc: message to be dispatched.
	*/
	void operator()(IN DataContext& dc);
	void handle_client(OUT Context& c, IN  TcpPeer& peer);
	void handle_server(OUT Context& c, IN  TcpPeer& peer);

public:
	// receive mode event.
	std::function<void(TcpPeer::ptr&, DataContext&)> m_recv;
};


////////////////////////////////////////////////////////////////////////////////
// tcp client dispatch server.
class DispatchServer : public eco::MessageServer<DataContext, DispatchHandler>
{
public:
	typedef std::function<void(IN Context&)> HandlerFunc;
	inline void register_handler(IN int id, IN HandlerFunc&& hf)
	{
		message_handler().set_dispatch(id, hf);
	}

	inline void register_default(IN HandlerFunc&& hf)
	{
		message_handler().set_default(hf);
	}
};


////////////////////////////////////////////////////////////////////////////////
// tcp server dispatch server.
class DispatchServerPool :
	public eco::MessageServerPool<DataContext, DispatchHandler>
{
public:
	typedef std::function<void(IN Context&)> HandlerFunc;
	inline void register_handler(IN int id, IN HandlerFunc&& hf)
	{
		message_handler().set_dispatch(id, hf);
	}

	inline void register_default(IN HandlerFunc&& hf)
	{
		message_handler().set_default(hf);
	}
};
typedef DispatchServerPool::MessageWorker MessageWorker;


////////////////////////////////////////////////////////////////////////////////
}}
#endif