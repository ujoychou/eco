#ifndef ECO_NET_TCP_ACCEPTOR_H
#define ECO_NET_TCP_ACCEPTOR_H
/*******************************************************************************
@ name


@ function


@ exception


@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-12.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/net/TcpPeer.h>


namespace eco{;
namespace net{;
class TcpServer;
class IoService;
////////////////////////////////////////////////////////////////////////////////
class TcpAcceptor
{
	ECO_OBJECT_API(TcpAcceptor);
public:
	// network server.
	void set_server(TcpServer& server);
	TcpServer& server();

	// register tcp acceptor handler.
	typedef std::function<void(TcpPeer::ptr& peer, bool error)> TcpOnAccept;
	void register_on_accept(IN TcpOnAccept handler);

	// get io service
	IoService* get_io_service();

	/*@ start accept.
	* @ para.port: listen port which to accept new client connection.
	*/
	void listen(
		IN  const uint16_t port,
		IN  const uint16_t io_server_size);

	/*@ async accept peer.*/
	void async_accept();

	/*@ stop accept tcp server peer.*/
	void stop();
	bool stopped() const;
	bool running() const;

	/*@ join this server threads.*/
	void join();
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif
