#include "PrecHeader.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <eco/net/TcpAcceptor.h>
////////////////////////////////////////////////////////////////////////////////
#include <mutex>
#include <eco/net/asio/WorkerPool.h>
#include "../TcpServer.ipp"


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class TcpPeerPool : public eco::Object<TcpPeerPool>
{
public:
	inline TcpPeerPool() : m_server(0)
	{}

	inline void start(TcpServer::Impl* server)
	{
		m_server = server;
		m_recycle = std::bind(&TcpPeerPool::recycle, 
			this, std::placeholders::_1);
		m_queue.run("peer_pool", 1);
		m_queue.set_message_handler(std::bind(
			&TcpPeerPool::release, this, std::placeholders::_1));
	}

	inline void clear()
	{
		std::lock_guard<std::mutex> lock(m_queue.mutex());
		m_buffer.clear();
	}

	TcpPeer::ptr make(asio::Worker* io_srv, void* msg_srv)
	{
		std::lock_guard<std::mutex> lock(m_queue.mutex());
		// release io stopped peer.
		while (!m_buffer.empty() && m_buffer.back()->stopped())
		{
			m_buffer.pop_back();
		}

		// create new peer.
		if (m_buffer.empty())
		{
			auto* io = (IoWorker*)io_srv;
			std::auto_ptr<TcpPeer> ptr(new TcpPeer(
				io, msg_srv, &m_server->m_peer_handler));
			m_buffer.push_back(std::move(ptr));
		}
		TcpPeer::ptr peer(m_buffer.back().release(), m_recycle);
		peer->impl().restart((MessageWorker*)msg_srv, peer);
		m_buffer.pop_back();
		return peer;
	}

	inline void recycle(IN TcpPeer* peer)
	{
		m_queue.post(peer);
	}

	inline void release(IN TcpPeer* peer)
	{
		// release peer user defined data.
		std::auto_ptr<TcpPeer> ptr(peer);
		try
		{
			eco::this_thread::lock().set_object(ptr->impl().id());
			ptr->impl().release();
		}
		catch (std::exception& e)
		{
			ECO_ERROR << NetLog(ptr->id(), __func__) <= ptr->ip() <= e.what();
		}
		
		// release buffer peer algorithm.
		// b_size = max(c_size / 10, 5);
		if (!ptr->stopped())
		{
			auto size = m_server->get_peer_size() / 10;
			if (size < 5) size = 5;
			std::lock_guard<std::mutex> lock(m_queue.mutex());
			m_buffer.push_back(std::move(ptr));
			while (m_buffer.size() > size)
			{
				m_buffer.pop_back();
			}
		}
	}

public:
	// connecntion pool.
	TcpServer::Impl* m_server;
	std::vector<std::auto_ptr<TcpPeer> > m_buffer;
	eco::MessageServer<TcpPeer*> m_queue;
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
	TcpServer::Impl* m_server;

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
			m_server->m_dispatch_pool.attach()));
		pr->impl().m_id = m_server->make_session_id();
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
		m_peer_pool.clear();
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
			eco::Error e(ec.value(), ec.message());
			m_on_accept(pr, &e);
		}
		else
		{
			m_on_accept(pr, nullptr);
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
void test_close_io_service(TcpAcceptor::Impl* impl)
{
	impl->m_worker_pool.stop_test(0, false);
}


////////////////////////////////////////////////////////////////////////////////
}}