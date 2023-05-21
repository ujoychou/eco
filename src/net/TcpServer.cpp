#include "Prec.h"
#include <eco/net/TcpServer.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/log/Log.h>
#include <eco/net/TcpOption.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class TcpServer::Impl : public TcpEvent
{
public:
    // tcp server setting
    TcpOptionServer     option;         // tcp server option
    protocol::Family    family;         // tcp protocol family

    // tcp socket handler
    TcpAccept::ptr      accept;         // accept
    TcpWorker           worker_accept;  // thread: accept
    TcpWorkerPool       worker_socket;  // thread pool: socket handler
    TcpRouterPool       router_pool;    // thread pool: message handler
    TcpSocketManager    manager;        // tcp socket management

    // tcp socket timer
    Timing::Timer       timer_close;
    Timing::Timer       timer_heart;

    // tcp server event
    EventClose          on_user_close;
    EventAccept         on_user_accept;

    inline Impl()
    {
        this->accept = std::make_shared<TcpAccept>(
            this->option, this->worker_accept, this->worker_socket);
    }

    // tcp event: accept client socket
    virtual void on_accept(TcpSocket::ref& sock, bool err) override
    {
        if (err)
        {
            ECO_FUNC(error) << eco::Error();
            return;
        }
        
        // max connections
        if (this->manager.accept(sock))
        {
            // socket: protocol-latest, ts, state, option
            sock.set_option(this->option);
            sock.set_protocol(this->family.protocol_latest());
            sock.set_state_connected(kconnected);
            if (this->on_user_accept)
            {
                this->on_user_accept(sock->id);
            }
        }
        else
        {
            sock->close();
        }
    }

    void decode()
    {
        eco::Result on_decode_head(MessageTcp& tcp, const char* buff, uint32_t size)
	{
		/*@ eco::fail
		1.if message bytes not enough to check, need read more bytes.
		--A.version bytes not enough.
		--B.size bytes not enough.

		2.eco::error:
		if message is error message:
		--A.category invalid.
		--B.get protocol invalid by version.
		--C.message is max than max size.

		3.if message check edge success, and get message_size.
		--A.heartbeat message.
		--B.general message.
		*/

		// #.get message version & category.
		auto res = protocol_tcp()->decode(tcp, buff, size);	// (1.A)
		if (res != eco::ok) { return res; }

		if (!is_heartbeat(tcp.m_category) &&
			!eco::has(tcp.m_category, category_message))	// (2.A)
		{
			ECO_THIS_ERROR(e_message_category)
				< "category error: " < tcp.m_category;
			return eco::error;
		}

		// #.get protocol by head version.
		tcp.m_protocol = protocol(tcp.m_version);
		if (tcp.m_protocol == nullptr)	// (2.B)
		{
			ECO_THIS_ERROR(e_protocol_invalid)
				< "protocol ver error: " < tcp.m_version;
			return eco::error;
		}
		// message size = head_size + size_size + data_size.
		if (!tcp.m_protocol->decode_tcp(head, buff, size))	// (1.B)
		{
			return eco::fail;
		}
		if (tcp.message_size() > size)							// (1.B)
		{
			return eco::fail;
		}
		if (tcp.message_size() > tcp.m_protocol->max_size())	// (2.C)
		{
			ECO_THIS_ERROR(e_message_overszie)
				< "message size over max size: "
				< tcp.message_size() < '>' < prot->max_size();
			return eco::error;
		}
		return eco::ok;		// (3.A/B)
	}
    

    void on_decode_head(MessageTcp& head, const char* buff, uint32_t size)
    {
        /*@forbid memory explosion and crush when [on_decode_head] aways
        return false, and head.message_size() is (A)==0 or (B)>size.
        1.keep message size algor right when get head_size & data_size.(A)
        2.set message max size in [on_decode_head] & protocol.(B)
        3.set tcp server & client max size.(B)
        4.set tcpconnector buff max size.(B)
        note important: all up 4 point must satisfied.
        */
        eco::Result res = this->on_decode_head
            ? this->on_decode_head(head, buff, size);
            : this->family.on_decode_head(head, buff, size);
        if (res != eco::ok) { return res; }

        // message is valid.
        if (head.message_size() == 0)
        {
            ECO_THIS_ERROR(e_message_decode) < "decode head message_size=0";
            return eco::error;
        }
        if (head.message_size() > option().max_byte_size())
        {
            ECO_THIS_ERROR(e_message_overszie)
                < "message size > tcp option max_byte_size: "
                < head.message_size() < '>' < option().max_byte_size();
            return eco::error;
        }
        return (head.message_size() <= size) ? eco::ok : eco::fail;
    }

    virtual void on_read(TcpSocket::ref& sock, String& buff, bool err) override
    {
        /*@recv scene.
		1.peer destruct. (close)
		2.peer has closed with some error. (close)

		3.recv a empty message.
		4.recv one or more completed message.
		  exp: "1 message; 2 message; 5 message."
		5.recv uncomplete message.
		  exp: "0.5 message; 1.4 message; 3.3 message."

		6.recv message format error. (close)
		  exp:"message oversize; category invalid; protocol unsupported."
		7.recv message means error. (close)
		  exp:"websocket frame==0x8 means close."
		*/
        if (err)
        {
            ECO_FUNC(error) << eco::Error();
            return ;
        }

		// append prev message
        buff.push_front(sock->buffer);
		for (size_t i = 0; i < buff.size(); i += tcp.size())
		{
			uint32_t size = buff.size() - i;
			if (size == 0)				// (3/4)
			{
				buff.clear();
				break;
			}

			MessageTcp tcp;
			eco::Result result = decode_head(tcp, &buff[i], size);
			if (result == eco::error)	// (6/7)
			{
				close_error_peer(sock);
				return;
			}
			if (result == eco::fail)	// (5) uncomplete
			{
                sock->buffer.assign(&buff[i], size);
				break;
			}
			// (3/4)
			eco::String buff(&buff[i], tcp.size());
            tcp.socket = sock;
            tcp.buffer.assign(&buff[i], tcp.size());
            tcp.protocol = this->family.protocol(tcp.version);
			sock->router.post(std::move(tcp));
		}
	}

public:
    void on_handle(MessageTcp& tcp)
    {
        // decode meta
        MessageMeta meta;
        if (!tcp.decode_meta(meta))
        {
            return;
        }

        // create session and decode message
        TcpSession::ptr sess = session_make(data, meta);
        if (!sess->on_decode())
        {
            return;
        }

        // session check
        if (sess->check_app_ready() && !app_ready())
        {
            return;
        }
        if (sess->check_auth() && !sess->auth())
        {
            return;
        }

        // session logging
        if (sess->log_sev_req() > 0)
        {
            ECO_REQ(*sess);
            return;
        }
        
        // session event
        session_handle(sess);
    }
};


////////////////////////////////////////////////////////////////////////////////
void TcpServer::start()
{
    // request handler thread
    impl->router_pool.set_capacity(impl->option.message_capacity());
    impl->router_pool.set_handler();
    impl->router_pool.run("net_router", impl->option.handle_thread_size());

    // accept: start accept client.
    impl->worker_accept.run("net_accept");
    impl->worker_socket.run("net_socket", impl->option.socket_thread_size());
    impl->accept.listen(*impl);
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net)
ECO_NS_END(eco)