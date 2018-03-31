#ifndef ECO_NET_TCP_PEER_H
#define ECO_NET_TCP_PEER_H
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
#include <eco/Project.h>
#include <eco/net/Address.h>
#include <eco/net/TcpState.h>
#include <eco/net/protocol/Protocol.h>
#include <eco/net/protocol/TcpProtocol.h>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class IoService;
class TcpPeer;
class Context;
class TcpPeerHandler
{
public:
	// when peer has connected to server.
	virtual void on_connect()
	{}

	// when peer has received a message data bytes.
	virtual void on_read(IN void* peer, IN eco::String& data) = 0;

	// when peer has sended a data, async notify sended data size.
	virtual void on_send(IN void* peer, IN const uint32_t size)
	{}

	// when peer has been closed.
	virtual void on_close(IN const ConnectionId peer_id)
	{}

	// get protocol head.
	virtual ProtocolHead& protocol_head() const = 0;

	// whether is a websocket.
	virtual bool websocket() const = 0;

	// websocket server key.
	virtual const char* websocket_key() const
	{
		return "";
	}
};


////////////////////////////////////////////////////////////////////////////////
class ConnectionData;
class ECO_API TcpPeer
{
	ECO_OBJECT_API(TcpPeer);
public:
	/*@ factory method to create a new peer.
	* @ para.service: io service of peer.
	*/
	static TcpPeer::ptr make(
		IN IoService* io,
		IN TcpPeerHandler* handler);

	// constructor
	TcpPeer(
		IN IoService* io,
		IN TcpPeerHandler* hdl);

	TcpPeerHandler& handler();

	// tcp peer identity which is the address of connector.
	ConnectionId get_id() const;

	// tcp remote client peer ip.
	const eco::String get_ip() const;

	// tcp peer connection state.
	TcpState& state();
	const TcpState& get_state() const;
	// set tcp peer state to connected.
	void set_connected();

	// set tcp peer option
	void set_option(IN bool no_delay);

	// get peer data.
	ConnectionData* data();

	// async connect to server address.
	void async_connect(IN const Address& addr);

	// async recv message from peer.
	void async_recv();
	void async_recv_shakehand();
	void async_recv_by_server();

	// close peer.
	void close();

	// close peer and notify peer handler.
	void close_and_notify(IN const eco::Error* e);

	// async send string message.
	void async_send(IN eco::String& data, IN const uint32_t start);

	// async send meta message.
	void async_send(IN const MessageMeta& meta, IN Protocol& prot);

	// async response message.
	void async_response(
		IN Codec& codec,
		IN const uint32_t type,
		IN const Context& context,
		IN Protocol& prot,
		IN const bool last,
		IN const bool encrypted);
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif