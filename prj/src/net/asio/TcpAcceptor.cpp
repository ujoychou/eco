#include "PrecHeader.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <eco/net/TcpAcceptor.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/net/asio/WorkerPool.h>
#include "../TcpPeer.ipp"
#include "../TcpServer.ipp"



namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpAcceptor::Impl
{
public:
	// asio acceptor listening service port to accept client.
	std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;

	// aiso io service.
	eco::net::asio::Worker m_worker;
	eco::net::asio::WorkerPool m_worker_pool;

	// tcp handler
	TcpOnAccept m_on_accept;

	// tcp server;
	TcpServer::Impl* m_server;

public:
	inline void init(IN TcpAcceptor&)
	{
		m_server = nullptr;
	}

	/*@ async accept peer.*/
	inline void async_accept()
	{
		TcpPeer::ptr pr(TcpPeer::make(
			(IoService*)m_worker_pool.get_io_service(), m_server));
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
		m_worker.run();
		m_acceptor.reset(new tcp::acceptor(*m_worker.get_io_service()));
		// bind the acceptor address.
		tcp::endpoint endpoint(tcp::v4(), port);
		m_acceptor->open(endpoint.protocol());
		m_acceptor->set_option(tcp::acceptor::reuse_address(true));
		m_acceptor->bind(endpoint);
		m_acceptor->listen();

		// start io services for connections.
		m_worker_pool.run(io_server_size);

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
		IN const boost::system::error_code& e)
	{
		if (e)
		{
			eco::Error error(e.message(), e.value());
			m_on_accept(pr, &error);
		}
		else
		{
			pr->set_option(m_server->m_option.no_delay());
			m_on_accept(pr, nullptr);
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_MOVABLE_IMPL(TcpAcceptor);
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

void TcpAcceptor::join()
{
	impl().join();
}

void TcpAcceptor::set_server(TcpServer& server)
{
	impl().m_server = &server.impl();
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
}}