#include "Pch.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <eco/net/TcpAcceptor.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/net/asio/WorkerPool.h>
#include "../TcpServer.ipp"


ECO_NS_BEGIN(eco);
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpPeerPool : public eco::Object<TcpPeerPool>
{
public:
	inline TcpPeerPool() : m_server(0)
	{}

	inline void start(TcpServer* server)
	{
		m_server = &server->impl();
		m_recycle = std::bind(&TcpPeerPool::recycle, 
			this, std::placeholders::_1);
		m_destroyer.run("peer_pool", 1);
		m_destroyer.set_message_handler(std::bind(
			&TcpPeerPool::release, this, std::placeholders::_1));
	}

	inline void stop()
	{
		m_destroyer.stop();
	}

	TcpPeer::ptr make(asio::Worker* io_srv, void* msg_srv)
	{
		TcpPeer::ptr peer(new TcpPeer((IoWorker*)io_srv,
			msg_srv, &m_server->m_peer_handler), m_recycle);
		peer->impl().prepare(peer);
		return peer;
	}

	inline void recycle(IN TcpPeer* peer)
	{
		m_destroyer.post(peer);
	}

	inline void release(IN TcpPeer* peer)
	{
		auto id = peer->id();
		ECO_LOG(key, "recycle_peer") < id;

		// release peer user defined data.
		try
		{
			eco::this_thread::lock().set_object(id);
			delete peer;	// std::auto_ptr<TcpPeer>
		}
		catch (std::exception& e)
		{
			ECO_ERROR < NetLog(id, __func__) <= e.what();
		}
	}

public:
	// connecntion pool.
	TcpServer::Impl* m_server;
	eco::MessageServer<TcpPeer*> m_destroyer;
	std::function<void(TcpPeer*)> m_recycle;
};


////////////////////////////////////////////////////////////////////////////////
class TcpAcceptor::Impl
{
public:
	// asio acceptor listening service port to accept client.
	std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;

	// aiso io service.
	eco::net::asio::Worker m_worker;
	eco::net::asio::WorkerPool m_worker_pool;
	eco::net::TcpPeerPool m_peer_pool;

	// tcp handler
	TcpOnAccept m_on_accept;

	// tcp server;
	TcpServer* m_server;

public:
	inline void init(IN TcpAcceptor&)
	{
		m_server = nullptr;
	}

	/*@ async accept peer.*/
	inline void async_accept()
	{
		TcpPeer::ptr pr(m_peer_pool.make(
			m_worker_pool.get_io_worker(),
			m_server->impl().m_dispatch_pool.attach()));
		pr->impl().m_id = m_server->impl().make_session_id();
		m_acceptor->async_accept(
			*(boost::asio::ip::tcp::socket*)(pr->impl().socket()),
			boost::bind(&Impl::on_accept, this,
				pr, boost::asio::placeholders::error));
	}

	inline void listen(
		IN const uint16_t port,
		IN const uint16_t io_server_size)
	{
		using namespace boost::asio::ip;

		// start service thread.
		m_worker.run("listen");
		m_acceptor.reset(new tcp::acceptor(*m_worker.get_io_service()));
		m_peer_pool.start(m_server);
		// bind the acceptor address.
		tcp::endpoint endpoint(tcp::v4(), port);
		m_acceptor->open(endpoint.protocol());
		m_acceptor->set_option(tcp::acceptor::reuse_address(true));
		m_acceptor->bind(endpoint);
		m_acceptor->listen();

		// start io services for connections.
		m_worker_pool.run(io_server_size, "worker");

		// accept client peer.
		async_accept();
	}

	inline void stop()
	{
		if (m_acceptor != nullptr)
		{
			m_acceptor->close();
			// destroy acceptor before worker stop.
			m_acceptor.reset();
		}
		// stop worker.
		m_peer_pool.stop();
		m_worker.stop();
		m_worker_pool.stop();
	}

	inline void join()
	{
		m_worker.join();
		m_worker_pool.join();
	}

	/*@ when accepted a client connection.*/
	inline void on_accept(
		IN TcpPeer::ptr& pr,
		IN const boost::system::error_code& ec)
	{
		if (ec)
		{
			ECO_THIS_ERROR(ec.value()) << ec.message();
			m_on_accept(pr, true);
		}
		else
		{
			m_on_accept(pr, false);
			async_accept();		// accept next tcp_connection.
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_OBJECT_IMPL(TcpAcceptor);
void TcpAcceptor::listen(
	IN  const uint16_t port,
	IN  const uint16_t io_server_size)
{
	impl().listen(port, io_server_size);
}
void TcpAcceptor::async_accept()
{
	impl().async_accept();
}
void TcpAcceptor::stop()
{
	impl().stop();
}
bool TcpAcceptor::stopped() const
{
	return impl().m_worker.stopped();
}
bool TcpAcceptor::running() const
{
	return impl().m_worker.running();
}
void TcpAcceptor::join()
{
	impl().join();
}
void TcpAcceptor::set_server(TcpServer& server)
{
	impl().m_server = &server;
}
TcpServer& TcpAcceptor::server()
{
	return *impl().m_server;
}
void TcpAcceptor::register_on_accept(IN TcpOnAccept v)
{
	impl().m_on_accept = v;
}
IoService* TcpAcceptor::get_io_service()
{
	return (IoService*)impl().m_worker.get_io_service();
}


////////////////////////////////////////////////////////////////////////////////
void test_close_io_service(TcpAcceptor::Impl* impl)
{
	impl->m_worker_pool.stop_test(0, false);
}


////////////////////////////////////////////////////////////////////////////////
}}