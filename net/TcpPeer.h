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
#include <eco/net/Address.h>
#include <eco/net/TcpState.h>
#include <eco/net/TcpOption.h>
#include <eco/net/protocol/TcpProtocol.h>


namespace eco{;
namespace net{;
class IoWorker;
class TcpPeer;
class Context;
class ProtocolFamily;
////////////////////////////////////////////////////////////////////////////////
// tcp owner: tcp client or tcp server.
class TcpOwner
{
public:
	inline TcpOwner() : m_server(false), m_owner(0)
	{}

	inline TcpOwner(void* owner, bool server)
	{
		set(owner, server);
	}

	inline void set(void* owner, bool server)
	{
		m_owner = owner;
		m_server = server;
	}

	inline void clear()
	{
		m_owner = 0;
		m_server = 0;
	}

	void*		m_owner;
	uint32_t	m_server;
};


////////////////////////////////////////////////////////////////////////////////
class TcpPeerHandler
{
public:
	void* m_owner;

	// protocol.
	const ProtocolFamily* m_protocol;

	// websocket server key.
	const char* m_websocket_key;

	// get protocol head.
	const TcpOption* m_option;

	// when peer has connected to server.
	// m_on_connect(error)
	std::function<void(IN const Error*)> m_on_connect;

	// when peer has sended a data, async notify sended data size.
	// m_on_send(peer, size)
	std::function<void(IN void*, IN uint32_t)> m_on_send;

	// when peer has read a completed message.
	// m_on_read(peer, head, data)
	std::function<void(IN void*, IN MessageHead&, IN String&)> m_on_read;

	// when recv data size, check the bytes tcp size edge.
	// m_on_decode_head(head, byte, size, error)
	std::function<bool(IN MessageHead&, IN const char*, IN uint32_t)>
		m_on_decode_head;

	// when peer has been closed.
	// m_on_close(peer_id, erase_peer)
	std::function<void(IN SessionId, IN const eco::Error& e, 
		IN bool erase_peer)> m_on_close;
};


////////////////////////////////////////////////////////////////////////////////
class ConnectionData;
class ECO_API TcpPeer
{
	ECO_OBJECT_API(TcpPeer);
public:
	/*@ used by tcp client to create a new peer.
	* @ para.service: io service of peer.
	*/
	static TcpPeer::ptr make(IoWorker* io, void* msg, TcpPeerHandler* hdl);
	TcpPeer(IoWorker* io_server, void* msg_server, TcpPeerHandler* hdl);
	TcpPeerHandler& handler();

	// init socket option.
	void init_option(const TcpOption& opt);

	// tcp peer identity which is the address of connector.
	SessionId id() const;
	const char* user() const;
	const char* lang() const;

	// tcp remote client peer ip.
	eco::String ip() const;
	uint32_t port() const;

	// io stopped.
	bool stopped() const;

	// tcp peer connection state.
	const TcpState& state() const;

	// tcp peer connection state.
	eco::atomic::State& data_state();

	// get peer data.
	ConnectionData* data();

	// authorize user and language.
	void authorize(IN const char* user, IN const char* lang);

	// whether peer has been authorized.
	bool authorized() const;

	// async connect to server address.
	void async_connect(IN const Address& addr);

	// close peer.
	void close();

	// close peer and notify peer handler.
	void close_and_notify(IN const eco::Error& e);

	// send string message.
	void send(IN eco::String& data, IN uint32_t start);

	// send meta message.
	void send(IN const MessageMeta& meta, IN Protocol& prot);

	// response message.
	void response(IN MessageMeta& m, IN Protocol& p, IN const Context& c);
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif