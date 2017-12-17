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
#include <eco/net/SessionData.h>
#include <eco/net/protocol/Protocol.h>
#include <eco/net/protocol/TcpProtocol.h>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
enum 
{
	tcp_peer_host_server	= 1,
	tcp_peer_host_client	= 2,
};
typedef uint32_t TcpPeerHost;


////////////////////////////////////////////////////////////////////////////////
class IoService;
class TcpPeer;
class TcpPeerHandler
{
public:
	// when peer has connected to server.
	virtual void on_connect()
	{}

	// when peer has received a message data bytes.
	virtual void on_read(
		IN void* peer,
		IN eco::String& data) = 0;

	// when peer has sended a data, async notify sended data size.
	virtual void on_send(
		IN void* peer,
		IN const uint32_t size)
	{}

	// when peer has been closed.
	virtual void on_close(
		IN uint64_t peer_id)
	{}

public:
	// get tcp peer host.
	virtual TcpPeerHost get_host() const = 0;

	// get protocol.
	virtual ProtocolHead* protocol_head() const = 0;

	// get protocol.
	virtual Protocol* protocol(IN const uint32_t version) const = 0;

	// find exist session.
	virtual SessionData::ptr find_session(IN const SessionId id) const
	{
		return SessionData::ptr();
	}

	// erase session.
	virtual void erase_session(IN const SessionId id)
	{}
};


////////////////////////////////////////////////////////////////////////////////
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

	// tcp peer callback handler.
	void set_handler(IN TcpPeerHandler*);
	TcpPeerHandler& handler();

	// tcp peer identity which is the address of connector.
	int64_t get_id() const;

	// tcp peer connection state.
	TcpState& state();
	const TcpState& get_state() const;
	// set tcp peer state to connected.
	void set_connected();

	// async connect to server address.
	void async_connect(IN const Address& addr);

	// async recv message from peer.
	void async_recv();

	// async send message.
	void async_send(IN eco::String& data);
	void async_send(
		IN MessageMeta& meta,
		IN Protocol& prot,
		IN ProtocolHead& prot_head);

	// close peer.
	void close();

	// close peer and notify peer handler.
	void notify_close(IN const eco::Error* e);

public:
	// send request to server.
	inline void async_send(
		IN eco::net::Codec& codec,
		IN const uint32_t session_id,
		IN const uint32_t msg_type,
		IN const MessageModel model,
		IN const uint32_t request_id,
		IN const uint32_t category = category_message)
	{
		IN eco::net::MessageMeta meta;
		eco::net::TcpProtocol prot;
		eco::net::TcpProtocolHead prot_head;
		meta.m_category = category;
		meta.set_session_id(session_id);
		meta.set_message_type(msg_type);
		meta.m_request_id = request_id;
		meta.m_model = model;
		meta.m_codec = &codec;
		async_send(meta, prot, prot_head);
	}

	// response to client.
	inline void async_request(
		IN eco::net::Codec& codec,
		IN const uint32_t session_id,
		IN const uint32_t msg_type,
		IN const uint32_t request_id,
		IN const uint32_t category = category_message)
	{
		async_send(codec, msg_type, session_id,
			model_req, request_id, category);
	}

	// response to client.
	inline void async_response(
		IN eco::net::Codec& codec,
		IN const uint32_t msg_type,
		IN const uint32_t session_id,
		IN const uint32_t request_id,
		IN const uint32_t category = category_message)
	{
		async_send(codec, msg_type, session_id,
			model_rsp, request_id, category);
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif