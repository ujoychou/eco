#include "PrecHeader.h"
#include <eco/net/TcpConnector.h>
////////////////////////////////////////////////////////////////////////////////
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <eco/Project.h>
#include <eco/log/Log.h>
#include <eco/thread/Mutex.h>
#include <eco/net/protocol/ProtocolHead.h>


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class TcpConnector::Impl
{
public:
	// socket for connection.
	boost::asio::ip::tcp::socket m_socket;

#ifndef ECO_WIN32
	// data queue.
	std::list<eco::String> m_send_msg;
	eco::Mutex m_send_msg_mutex;
#endif

	TcpConnectorHandler* m_handler;				// handler.
	std::weak_ptr<TcpPeer> m_peer_observer;		// parent peer.

public:
	Impl(IN boost::asio::io_service& srv)
		: m_socket(srv), m_handler(nullptr)
	{}

	~Impl()
	{}

	inline size_t get_id() const
	{
		return reinterpret_cast<size_t>(this);
	}

	inline const eco::String get_ip() const
	{
		std::string ip = m_socket.remote_endpoint().address().to_string();
		return eco::String(ip.c_str());
	}

	inline void async_connect(IN const Address& addr)
	{
		using namespace boost::asio::ip;
		close();

		// parse server address.
		boost::system::error_code ec;
		tcp::resolver resolver(m_socket.get_io_service());
		tcp::resolver::query query(
			addr.get_host_name(), addr.get_service_name());
		tcp::resolver::iterator it_endpoint = resolver.resolve(query, ec);
		if (ec)		// parse addr error.
		{
			EcoThrow(ec.value()) << ec.message();
		}

		// connect to server.
		boost::asio::async_connect(m_socket, it_endpoint,
			boost::bind(&Impl::on_connect, this, m_peer_observer,
			boost::asio::placeholders::error));
	}

	inline void set_option(IN bool delay)
	{
		boost::asio::ip::tcp::no_delay option(true);
		m_socket.set_option(option);
	}

	inline void on_connect(
		IN std::weak_ptr<TcpPeer>& peer_wptr,
		IN const boost::system::error_code& ec)
	{
		std::shared_ptr<TcpPeer> peer(peer_wptr.lock());
		if (peer == nullptr)
		{
			return;
		}

		if (ec) 
		{
			eco::Error e(ec.message(), ec.value());
			m_handler->on_connect(false, &e);
			return;
		}
		set_option(true);
		m_handler->on_connect(true, nullptr);
	}

	// close socket.
	inline void close()
	{
		boost::system::error_code ec;
		m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		m_socket.close(ec);
	}

	inline void async_read_until(
		IN const uint32_t data_size,
		IN const char* delimiter)
	{
		eco::String delim(delimiter);
		eco::String data(data_size);
		async_read_some(data, 0, delim);
	}

	inline void async_read_some(
		IN eco::String& data,
		IN uint32_t start_pos,
		IN eco::String& delimiter)
	{
		uint32_t size = data.size() - start_pos;
		char* buff = &data[start_pos];
		m_socket.async_read_some(
			boost::asio::buffer(buff, size),
			boost::bind(&Impl::on_read_until, this,
				eco::move(data), start_pos, eco::move(delimiter),
				m_peer_observer, boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	inline void on_read_until(
		IN eco::String& data,
		IN uint32_t start_pos,
		IN eco::String& delimiter,
		IN std::weak_ptr<TcpPeer>& peer_wptr,
		IN const boost::system::error_code& ec,
		IN size_t bytes_transferred)
	{
		std::shared_ptr<TcpPeer> peer(peer_wptr.lock());
		if (peer == nullptr)
		{
			return;
		}

		if (ec)
		{
			eco::Error e(ec.message(), ec.value());
			m_handler->on_read_data(data, &e);
			return;
		}

		// check whether reach the end.
		data.resize(start_pos + (uint32_t)bytes_transferred);
		if (!data.find_reverse(delimiter.c_str(), start_pos))
		{
			uint32_t start = data.size();
			data.resize(data.capacity());
			async_read_some(data, start, delimiter);
		}
		else
		{
			m_handler->on_read_data(data, nullptr);
		}
	}

public:
	inline void async_read_head(
		IN char* data,
		IN const uint32_t size)
	{
		boost::asio::async_read(m_socket,
			boost::asio::buffer(data, size),
			boost::bind(&Impl::on_read_head, this, data, size,
			m_peer_observer, boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	inline void on_read_head(
		IN char* data,
		IN const uint32_t size,
		IN std::weak_ptr<TcpPeer>& peer_wptr,
		IN const boost::system::error_code& ec,
		IN size_t bytes_transferred)
	{
		std::shared_ptr<TcpPeer> peer(peer_wptr.lock());
		if (peer == nullptr)
		{
			return;
		}

		if (ec)
		{
			eco::Error e(ec.message(), ec.value());
			m_handler->on_read_head(data, size, &e);
			return;
		}
		m_handler->on_read_head(data, size, nullptr);
	}

	inline void async_read_data(
		IN eco::String& data,
		IN const uint32_t head_size)
	{
		// eco::move(data) will clear eco::String, so can't use like:
		// boost::asio::buffer(&d[start], data.size() - start),
		char* d = &data[head_size];
		const uint32_t s = data.size() - head_size;
		boost::asio::async_read(m_socket,
			boost::asio::buffer(d, s),
			boost::bind(&Impl::on_read_data, this, eco::move(data),
			m_peer_observer, boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	inline void on_read_data(
		IN eco::String& data,
		IN std::weak_ptr<TcpPeer>& peer_wptr,
		IN const boost::system::error_code& ec,
		IN size_t bytes_transferred)
	{
		std::shared_ptr<TcpPeer> peer(peer_wptr.lock());
		if (peer == nullptr)
		{
			return;
		}

		if (ec) 
		{
			eco::Error e(ec.message(), ec.value());
			m_handler->on_read_data(data, &e);
			return;
		}
		
		m_handler->on_read_data(data, nullptr);
	}

public:
#ifdef ECO_WIN
	inline void async_write(
		IN eco::String& data,
		IN const uint32_t start)
	{
		// eco::move(data) will clear data(eco::String), so can't use like:
		// boost::asio::buffer(data.c_str(), data.size()),
		const char* d = &data[start];
		const uint32_t s = data.size() - start;
		boost::asio::async_write(m_socket,
			boost::asio::buffer(d, s),
			boost::bind(&Impl::on_write, this, eco::move(data), 
			m_peer_observer, boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	inline void on_write(
		IN eco::String& data,
		IN std::weak_ptr<TcpPeer>& peer_wptr,
		IN const boost::system::error_code& ec,
		IN size_t bytes_transferred)
	{
		std::shared_ptr<TcpPeer> peer(peer_wptr.lock());
		if (peer == nullptr)
		{
			return;
		}

		if (ec)
		{
			eco::Error e(ec.message(), ec.value());
			m_handler->on_write((uint32_t)bytes_transferred, &e);
			return;
		}
		m_handler->on_write((uint32_t)bytes_transferred, nullptr);
	}

#else
	inline void async_write(
		IN eco::String& data,
		IN const uint32_t start)
	{
		eco::Mutex::ScopeLock lock(m_send_msg_mutex);
		bool is_idle = m_send_msg.empty();

		// add to send msg queue.
		m_send_msg.push_back(std::move(data));

		// if io is idle, send message.
		if (is_idle)
		{
			eco::String sd(std::move(m_send_msg.front()));
			boost::asio::async_write(m_socket,
				boost::asio::buffer(&sd[0], sd.size()),
				boost::bind(&Impl::on_write, this, m_peer_observer,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}

	inline void on_write(
		IN std::weak_ptr<TcpPeer>& peer_wptr,
		IN const boost::system::error_code& ec,
		IN size_t bytes_transferred)
	{
		std::shared_ptr<TcpPeer> peer(peer_wptr.lock());
		if (peer == nullptr)
		{
			return;
		}

		if (ec)
		{
			eco::Error e(ec.message(), ec.value());
			m_handler->on_write((uint32_t)bytes_transferred, &e);
			return;
		} 

		m_handler->on_write((uint32_t)bytes_transferred, nullptr);

		// release sended data and send next msg.
		{
			eco::Mutex::ScopeLock lock(m_send_msg_mutex);
			m_send_msg.pop_front();
			if (!m_send_msg.empty())
			{
				eco::String sd(std::move(m_send_msg.front()));
				boost::asio::async_write(m_socket,
					boost::asio::buffer(&sd[0], sd.size()),
					boost::bind(&Impl::on_write, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
			}
		}
	}
#endif
};


ECO_IMPL(TcpConnector);
////////////////////////////////////////////////////////////////////////////////
TcpConnector::TcpConnector(IN IoService* srv)
{
	m_impl = new Impl(*(boost::asio::io_service*)srv);
}

TcpConnector::~TcpConnector()
{
	delete m_impl;
	m_impl = nullptr;
}

TcpSocket* TcpConnector::socket()
{
	return (TcpSocket*)&m_impl->m_socket;
}

void TcpConnector::set_option(IN bool delay)
{
	m_impl->set_option(delay);
}

void TcpConnector::register_handler(
	IN TcpConnectorHandler& handler,
	IN std::weak_ptr<TcpPeer>& peer)
{
	m_impl->m_handler = &handler;
	m_impl->m_peer_observer = peer;
}

void TcpConnector::async_connect(IN const Address& addr)
{
	m_impl->async_connect(addr);
}

void TcpConnector::close()
{
	m_impl->close();
}

void TcpConnector::async_read_head(
	IN char* data, IN const uint32_t head_size)
{
	m_impl->async_read_head(data, head_size);
}

void TcpConnector::async_read_data(
	IN eco::String& data, IN const uint32_t start)
{
	m_impl->async_read_data(data, start);
}

void TcpConnector::async_read_until(
	IN const uint32_t data_size, 
	IN const char* delimiter)
{
	m_impl->async_read_until(data_size, delimiter);
}

void TcpConnector::async_write(IN eco::String& data, IN const uint32_t start)
{
	m_impl->async_write(data, start);
}

size_t TcpConnector::get_id() const
{
	return impl().get_id();
}

const eco::String TcpConnector::get_ip() const
{
	return (eco::String&&)(impl().get_ip());
}

////////////////////////////////////////////////////////////////////////////////
}}