#include "PrecHeader.h"
#include <eco/net/TcpConnector.h>
////////////////////////////////////////////////////////////////////////////////
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <eco/Project.h>
#include <eco/thread/Mutex.h>
#include <eco/net/protocol/ProtocolHead.h>



////////////////////////////////////////////////////////////////////////////////
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

	// peer handler.
	TcpConnectorHandler* m_handler;

public:
	Impl(IN boost::asio::io_service& srv) : m_socket(srv)
	{}

	~Impl()
	{
		boost::system::error_code ec;
		m_socket.close(ec);
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
			boost::bind(&Impl::on_connect, this,
			boost::asio::placeholders::error));
	}

	inline void on_connect(
		IN const boost::system::error_code& ec)
	{
		if (ec) {
			eco::Error e(ec.message(), ec.value());
			m_handler->on_connect(false, &e);
		} else {
			m_handler->on_connect(true, nullptr);
		}
	}

	// close socket.
	inline void close()
	{
		boost::system::error_code ec;
		m_socket.close(ec);
	}

	inline void async_read_head(
		IN char* data,
		IN const uint32_t size)
	{
		boost::asio::async_read(m_socket,
			boost::asio::buffer(data, size),
			boost::bind(&Impl::on_read_head, this,
			data, size, boost::asio::placeholders::error));
	}

	inline void on_read_head(
		IN char* data,
		IN const uint32_t size,
		IN const boost::system::error_code& ec)
	{
		if (ec) {
			eco::Error e(ec.message(), ec.value());
			m_handler->on_read_head(data, size, &e);
		} else {
			m_handler->on_read_head(data, size, nullptr);
		}
	}

	inline void async_read_data(
		IN eco::String& data,
		IN const uint32_t start)
	{
		boost::asio::async_read(m_socket,
			boost::asio::buffer(&data[start], data.size() - start),
			boost::bind(&Impl::on_read_data, this,
			eco::move(data), boost::asio::placeholders::error));
	}

	inline void on_read_data(
		IN eco::String& data,
		IN const boost::system::error_code& ec)
	{
		if (ec) {
			eco::Error e(ec.message(), ec.value());
			m_handler->on_read_data(data, &e);
		}
		else {
			m_handler->on_read_data(data, nullptr);
		}
	}

#ifdef ECO_WIN
	inline void async_write(IN eco::String& data)
	{
		boost::asio::async_write(m_socket,
			boost::asio::buffer(&data[0], data.size()),
			boost::bind(&Impl::on_write, this,
			eco::move(data),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	inline void on_write(
		IN eco::String& data,
		IN const boost::system::error_code& ec,
		IN size_t bytes_transferred)
	{
		if (ec)
		{
			eco::Error e(ec.message(), ec.value());
			m_handler->on_write((uint32_t)bytes_transferred, &e);
		} 
		else 
		{
			m_handler->on_write((uint32_t)bytes_transferred, nullptr);
		}// end if.
	}

#else
	inline void async_write(IN eco::String& data)
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
				boost::bind(&Impl::on_write, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}

	inline void on_write(
		IN const boost::system::error_code& ec,
		IN size_t bytes_transferred)
	{
		if (ec)
		{
			eco::Error e(ec.message(), ec.value());
			m_handler->on_write((uint32_t)bytes_transferred, &e);
		} 
		else 
		{
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
		}// end if.
	}
#endif
};


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

void TcpConnector::register_handler(IN TcpConnectorHandler& handler)
{
	m_impl->m_handler = &handler;
}

void TcpConnector::async_connect(IN const Address& addr)
{
	m_impl->async_connect(addr);
}

void TcpConnector::close()
{
	m_impl->close();
}

void TcpConnector::async_read_head(IN char* data, IN const uint32_t head_size)
{
	m_impl->async_read_head(data, head_size);
}

void TcpConnector::async_read_data(IN eco::String& data, IN const uint32_t start)
{
	m_impl->async_read_data(data, start);
}

void TcpConnector::async_write(IN eco::String& data)
{
	m_impl->async_write(data);
}


////////////////////////////////////////////////////////////////////////////////
}}