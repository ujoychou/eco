#include "PrecHeader.h"
#include "TcpPeer.ipp"
////////////////////////////////////////////////////////////////////////////////
#include <eco/net/Ecode.h>
#include <eco/log/Log.h>
#include <eco/net/protocol/TcpProtocol.h>



namespace eco{;
namespace net{;
ECO_OBJECT_IMPL(TcpPeer);
////////////////////////////////////////////////////////////////////////////////
EcoThreadLocal char s_data_head[32] = {0};
inline void TcpPeer::Impl::async_recv()
{
	// ready to receive data head.
	uint32_t size = handler().protocol_head().head_size();
	m_connector->async_read_head(s_data_head, size);
}


////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::on_connect(
	IN bool is_connected,
	IN const eco::Error* e)
{
	if (!is_connected)
	{
		EcoNet(EcoError, *this, "connect", *e);
	}
	else
	{
		set_connected();			// set peer state.
		m_handler->on_connect();	// notify handler.
		async_recv();
	}
}


////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::on_read_head(
	IN char* data_head,
	IN const uint32_t head_size,
	IN const eco::Error* e)
{
	if (e != nullptr)	// if peerection occur error, close it.
	{
		EcoNet(EcoWarn, *this, "recv_head", *e);
		notify_close(e);
		return;
	}

	uint32_t data_size = handler().protocol_head().decode_data_size(data_head);
	// if recv message size is error.
	const uint32_t max_recv_size = 2 * 1024 * 1024;		// max data size < 2M.
	if (data_size > max_recv_size)
	{
		eco::Error e(e_message_overszie);
		e << "peer received a oversize message: " << data_size;
		EcoNet(EcoError, *this, "recv_head", e);
		notify_close(&e);
		return;
	}

	// when recv head from peer, means peer is alive.
	m_state.peer_live(true);

	// allocate memory for store coming data.
	eco::String data;
	data.resize(head_size + data_size);
	strncpy(&data[0], data_head, head_size);

	// empty message.
	if (data_size == 0)
	{
		m_handler->on_read(this, data);
		async_recv();	// recv next coming data.
		return;
	}

	// recv data from peer.
	m_connector->async_read_data(data, head_size);
}


////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::on_read_data(
	IN eco::String& data,
	IN const eco::Error* e)
{
	if (e != nullptr)	// if peer occur error, release it.
	{
		EcoNet(EcoWarn, *this, "recv_data", *e);
		notify_close(e);
		return;
	}

	// when recv data message from peer, means peer is active.
	m_state.peer_active(true);

	// post data message to tcp server.
	uint32_t head_size = m_handler->protocol_head().head_size();
	m_handler->on_read(this, data);

	// recv next coming data.
	async_recv();
}


////////////////////////////////////////////////////////////////////////////////
void TcpPeer::Impl::on_write(IN const uint32_t size, IN const eco::Error* e)
{
	if (e != nullptr)
	{
		notify_close(e);
		return;
	}

	// notify tcp handler.
	m_handler->on_send(this, size);
}


//##############################################################################
//##############################################################################
TcpPeer::ptr TcpPeer::make(
	IN IoService* io,
	IN TcpPeerHandler* hdl)
{
	TcpPeer::ptr peer(new TcpPeer);
	peer->impl().m_peer_observer = peer;
	peer->impl().make(io, hdl);
	return peer;
}
void TcpPeer::set_handler(IN TcpPeerHandler* v)
{
	impl().set_handler(v);
}
TcpPeerHandler& TcpPeer::handler()
{
	return impl().handler();
}
int64_t TcpPeer::get_id() const
{
	return impl().get_id();
}
TcpState& TcpPeer::state()
{
	return impl().state();
}
const TcpState& TcpPeer::get_state() const
{
	return impl().get_state();
}
void TcpPeer::set_connected()
{
	impl().set_connected();
}
void TcpPeer::set_option(IN bool no_delay)
{
	impl().m_connector->set_option(no_delay);
}
void TcpPeer::async_connect(IN const Address& addr)
{
	impl().async_connect(addr);
}
void TcpPeer::async_recv()
{
	impl().async_recv();
}
void TcpPeer::async_send(IN eco::String& data)
{
	impl().async_send(data);
}
void TcpPeer::async_send(
	IN MessageMeta& meta, IN Protocol& prot, IN ProtocolHead& prot_head)
{
	impl().async_send(meta, prot, prot_head);
}
void TcpPeer::close()
{
	impl().close();
}
void TcpPeer::notify_close(IN const eco::Error* e)
{
	impl().notify_close(e);
}


////////////////////////////////////////////////////////////////////////////////
}}