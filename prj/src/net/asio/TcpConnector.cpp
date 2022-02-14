#include "Pch.h"
#include <eco/net/TcpConnector.h>
////////////////////////////////////////////////////////////////////////////////
#include <deque>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <eco/rx/RxImpl.h>
#include <eco/net/asio/Worker.h>
#include "../TcpPeer.ipp"


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class MessageMonitor
{
public:
	inline MessageMonitor() : m_last(0) {}

	inline bool change_much(uint32_t size) const
	{
		// grow up or blow up 20%;
		int delta = int(size) - int(m_last);
		if (delta < 0) delta *= -1;
		return delta * 5 > int(m_last);
	}

	inline void warn(const char* name, size_t id, uint32_t size, uint32_t capacity)
	{
		if (size * 2 >= capacity)
		{
			if (change_much(size))
			{
				m_last = size;
				ECO_ERROR << id <= name <= size < '/' < capacity;
			}
		}
		else if (size * 5 >= capacity)
		{
			if (change_much(size))
			{
				m_last = size;
				ECO_WARN << id <= name <= size < '/' < capacity;
			}
		}
		else
		{
			m_last = 0;
		}
	}

	uint32_t m_last;
};


////////////////////////////////////////////////////////////////////////////////
class SendingQueue
{
public:
	class String
	{
	public:
		inline String(eco::String& d, uint32_t s)
			: m_start(s), m_data(std::move(d)) {}

		inline String(String&& s) 
			: m_start(s.m_start), m_data(std::move(s.m_data))
		{}

		inline uint32_t size() const
		{
			return uint32_t(m_data.size()) - m_start;
		}

		inline const char* data() const
		{
			return &m_data[m_start];
		}

		uint32_t m_start;
		eco::String m_data;
	};

public:
	inline SendingQueue() : m_capacity(5000)
	{}

	inline void reset()
	{
		m_deque.clear();
	}

	inline void set_capacity(int capacity)
	{
		m_capacity = capacity;
	}

	inline void push_back(eco::String& msg, uint32_t start, size_t id)
	{
		if (m_deque.size() < m_capacity)
		{
			m_deque.push_back(String(msg, start));
		}
		m_monitor.warn("sending", id, (uint32_t)m_deque.size(), m_capacity);
	}

public:
	std::deque<String> m_deque;
	std_mutex m_mutex;
	uint32_t m_capacity;
	MessageMonitor m_monitor;
};


////////////////////////////////////////////////////////////////////////////////
class TcpConnector::Impl
{
public:
	// socket for connection.
	asio::Worker* m_worker;
	TcpConnectorHandler* m_handler;
	std::weak_ptr<TcpPeer> m_peer_observer;
	boost::asio::ip::tcp::socket m_socket;
	eco::String m_buffer;
#ifndef ECO_WIN
	SendingQueue m_send_msg;
#endif

public:
	inline Impl(IN asio::Worker& worker)
		: m_worker(&worker), m_handler(0)
		, m_socket(*worker.get_io_service())
		, m_buffer(1024, true)
	{}

	inline size_t get_id() const
	{
		return reinterpret_cast<size_t>(this);
	}

	inline bool stopped() const
	{
		return m_worker->stopped();
	}

	inline eco::String get_ip() const
	{
		std::string ip = m_socket.remote_endpoint().address().to_string();
		return eco::String(ip);
	}

	inline uint32_t get_port() const
	{
		return m_socket.remote_endpoint().port();
	}

	inline bool async_connect(IN const Address& addr)
	{
		using namespace boost::asio::ip;
		close();

		// parse server address.
		boost::system::error_code ec;
		tcp::resolver resolver(*m_worker->get_io_service());
		tcp::resolver::query query(addr.host_name(), addr.service_name());
		tcp::resolver::iterator it_endpoint = resolver.resolve(query, ec);
		if (ec)		// parse addr error.
		{
			ECO_THIS_ERROR(ec.value()) < ec.message();
			return false;
		}

		// connect to server.
		boost::asio::async_connect(m_socket, it_endpoint,
			boost::bind(&Impl::on_connect, this, m_peer_observer,
			boost::asio::placeholders::error));
		return true;
	}

	inline void on_connect(
		IN std::weak_ptr<TcpPeer>& peer_wptr,
		IN const boost::system::error_code& ec)
	{
		std::shared_ptr<TcpPeer> peer(peer_wptr.lock());
		if (peer == nullptr)
		{
			ECO_FUNC(info) << "peer is empty.";
			return;
		}

		if (ec) 
		{
			ECO_THIS_ERROR(ec.value()) < ec.message();
			m_handler->on_connect(true);
			return;
		}
		m_handler->on_connect(false);
	}

	// release connector data.
	inline void release();

	// close socket.
	inline void close()
	{
		boost::system::error_code ec;
		m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		m_socket.close(ec);
		release();	// release buffer when client closed.
	}

public:
	// resize buffer.
	inline void reverse_buffer()
	{
		if (m_buffer.size() == m_buffer.capacity())
		{
			m_buffer.reserve(m_buffer.capacity() * 2);
		}
	}

	inline void close_error_peer(MessageHead& head, String& buff)
	{
		m_handler->on_read(head, buff, true);
	}

	inline std::shared_ptr<TcpPeer> check_error(
		IN std::weak_ptr<TcpPeer>& peer_wptr,
		IN const boost::system::error_code& ec)
	{
		std::shared_ptr<TcpPeer> peer(peer_wptr.lock());
		if (peer == nullptr)
		{
			ECO_FUNC(info) << "peer is empty.";
			return peer;
		}
		if (ec)		// error tcp peer will close this socket.
		{
			ECO_THIS_ERROR(ec.value()) < ec.message();
			close_error_peer(eco::lvalue(MessageHead()), m_buffer);
			peer.reset();
		}
		return peer;
	}

public:
	inline void async_read_until(IN eco::String& delimiter)
	{
		// note: follow sentence is wrong, data will be null by eco::move.
		// boost::asio::buffer(&data[start_pos], size)
		uint32_t start = m_buffer.size();
		m_socket.async_read_some(
			boost::asio::buffer(&m_buffer[start], m_buffer.capacity() - start),
			boost::bind(&Impl::on_read_until, this,	eco::move(delimiter),
				m_peer_observer, boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	inline void on_read_until(
		IN eco::String& delimiter,
		IN std::weak_ptr<TcpPeer>& peer_wptr,
		IN const boost::system::error_code& ec,
		IN size_t bytes_transferred)
	{
		std::shared_ptr<TcpPeer> peer(check_error(peer_wptr, ec));
		if (peer == nullptr) return;

		// "last pos < delimiter size" find from 0, esle find_pos++;
		int rend = -1;
		if (m_buffer.size() > 0)
		{
			rend = m_buffer.size() - delimiter.size();
			if (rend < 0) rend = -1;
		}

		// find reverse.
		m_buffer.resize(m_buffer.size() + (uint32_t)bytes_transferred);
		uint32_t pos = m_buffer.find_reverse(delimiter.c_str(), -1, rend);
		if (pos == uint32_t(-1))
		{
			reverse_buffer();
			async_read_until(delimiter);
		}
		else
		{
			eco::String buff(m_buffer.c_str(), pos + delimiter.size());
			m_buffer.erase(0, buff.size());
			m_handler->on_read(eco::lvalue(MessageHead()), buff, false);
		}
	}

public:
	inline void async_read()
	{
		// note: follow sentence is wrong, data will be null by eco::move.
		// boost::asio::buffer(&data[0], size)
		uint32_t start = m_buffer.size();
		m_socket.async_read_some(
			boost::asio::buffer(&m_buffer[start], m_buffer.capacity() - start),
			boost::bind(&Impl::on_read_data, this, m_peer_observer,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	inline void on_read_data(
		IN std::weak_ptr<TcpPeer>& peer_wptr,
		IN const boost::system::error_code& ec,
		IN size_t bytes_transferred)
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
		std::shared_ptr<TcpPeer> peer(check_error(peer_wptr, ec));
		if (peer == nullptr) return;	// (1/2)

		// parse all completed message and notify handler.
		m_buffer.resize(m_buffer.size() + (uint32_t)bytes_transferred);
		for (uint32_t start = 0; true; )
		{
			uint32_t size = m_buffer.size() - start;
			if (size == 0)				// (3/4)
			{
				m_buffer.clear();
				break;
			}

			MessageHead head;
			eco::Result result = peer->impl().on_decode_head(
				head, &m_buffer[start], size);
			if (result == eco::error)	// (6/7)
			{
				close_error_peer(head, m_buffer);
				return;
			}
			if (result == eco::fail)	// (5) uncomplete
			{
				m_buffer.erase(0, start);	// move memory to recv more bytes.
				reverse_buffer();
				break;
			}
			// (3/4)
			eco::String buff(&m_buffer[start], head.message_size());
			m_handler->on_read(head, buff, false);	// notify handler
			start += head.message_size();	// check tcp size of next messge.
		}
		
		async_read();		// receive next message.
	}

public:
	// sync send message to peer.
	inline bool write(IN eco::String& data, IN const uint32_t start)
	{
		const char* d = &data[start];
		const uint32_t s = data.size() - start;
		boost::system::error_code ec;
		boost::asio::write(m_socket, boost::asio::buffer(d, s), ec);
		if (ec)
		{
			ECO_FUNC(debug) << "#" << ec.value() <= ec.message();
			return false;
		}
		return true;
	}

#ifdef ECO_WIN
	inline void async_write(IN eco::String& data, IN const uint32_t start)
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
		if (peer && peer->impl().connected())
		{
			if (ec)
			{
				ECO_THIS_ERROR(ec.value()) < ec.message();
				m_handler->on_write((uint32_t)bytes_transferred, true);
				return;
			}
			m_handler->on_write((uint32_t)bytes_transferred, false);
		}
	}

#else
	inline void async_write(IN eco::String& data, IN const uint32_t start)
	{
		// if io is idle, send message.
		std_lock_guard lock(m_send_msg.m_mutex);
		m_send_msg.push_back(data, start, get_id());
		if (m_send_msg.m_deque.size() == 1)
		{
			auto& msg = m_send_msg.m_deque.front();
			boost::asio::async_write(m_socket,
				boost::asio::buffer(msg.data(), msg.size()),
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
		if (peer && peer->impl().connected())
		{
			if (ec)
			{
				ECO_THIS_ERROR(ec.value()) < ec.message();
				m_handler->on_write((uint32_t)bytes_transferred, true);
				return;
			}
			m_handler->on_write((uint32_t)bytes_transferred, false);

			// release sended data and send next msg.
			std_lock_guard lock(m_send_msg.m_mutex);
			m_send_msg.m_deque.pop_front();
			if (!m_send_msg.m_deque.empty())
			{
				auto& msg = m_send_msg.m_deque.front();
				boost::asio::async_write(m_socket,
					boost::asio::buffer(msg.data(), msg.size()),
					boost::bind(&Impl::on_write, this, m_peer_observer,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
			}
		}
	}
#endif
};


ECO_IMPL_impl(TcpConnector);
////////////////////////////////////////////////////////////////////////////////
TcpConnector::TcpConnector(IN IoWorker* w)
{
	m_impl = new Impl(*(asio::Worker*)w);
}

TcpConnector::~TcpConnector()
{
	delete m_impl;
	m_impl = nullptr;
}

TcpSocket* TcpConnector::socket()
{
	return (TcpSocket*)&impl().m_socket;
}


////////////////////////////////////////////////////////////////////////////////
void TcpConnector::set_no_delay(IN bool delay)
{
	boost::asio::ip::tcp::no_delay option(true);
	impl().m_socket.set_option(option);
}
bool TcpConnector::no_delay() const
{
	boost::asio::ip::tcp::no_delay option;
	impl().m_socket.get_option(option);
	return option.value();
}
void TcpConnector::set_send_buffer_size(IN int size)
{
	if (size != 0)
	{
		boost::asio::socket_base::send_buffer_size option(size);
		impl().m_socket.set_option(option);
	}
}
int TcpConnector::get_send_buffer_size() const
{
	boost::asio::socket_base::send_buffer_size option;
	impl().m_socket.get_option(option);
	return option.value();
}
void TcpConnector::set_recv_buffer_size(IN int size)
{
	if (size != 0)
	{
		boost::asio::socket_base::receive_buffer_size option(size);
		impl().m_socket.set_option(option);
	}
}
int TcpConnector::get_recv_buffer_size() const
{
	boost::asio::socket_base::receive_buffer_size option;
	impl().m_socket.get_option(option);
	return option.value();
}
void TcpConnector::set_send_low_watermark(IN int size)
{
	if (size != 0)
	{
		boost::asio::socket_base::send_low_watermark option(size);
		impl().m_socket.set_option(option);
	}
}
int TcpConnector::get_send_low_watermark() const
{
	boost::system::error_code ec;
	boost::asio::socket_base::send_low_watermark option;
	impl().m_socket.get_option(option, ec);
	if (ec)
	{
		ECO_FUNC(error) < ec.message() < "#" < ec.value();
	}
	return option.value();
}
void TcpConnector::set_recv_low_watermark(IN int size)
{
	if (size != 0)
	{
		boost::asio::socket_base::receive_low_watermark option(size);
		impl().m_socket.set_option(option);
	}
}
int TcpConnector::get_recv_low_watermark() const
{
	boost::asio::socket_base::receive_low_watermark option;
	impl().m_socket.get_option(option);
	return option.value();
}


////////////////////////////////////////////////////////////////////////////////
void TcpConnector::register_handler(
	IN TcpConnectorHandler& handler,
	IN std::weak_ptr<TcpPeer>& peer)
{
	impl().m_handler = &handler;
	impl().m_peer_observer = peer;
}
bool TcpConnector::async_connect(IN const Address& addr)
{
	return impl().async_connect(addr);
}
void TcpConnector::close()
{
	impl().close();
}
void TcpConnector::async_read()
{
	impl().async_read();
}
void TcpConnector::async_read_until(IN const char* delimiter)
{
	eco::String s(delimiter);
	impl().async_read_until(s);
}
void TcpConnector::async_write(IN eco::String& data, IN const uint32_t start)
{
	impl().async_write(data, start);
}
bool TcpConnector::write(IN eco::String& data, IN const uint32_t start)
{
	return impl().write(data, start);
}
void TcpConnector::set_send_capacity(IN int capacity)
{
#ifndef ECO_WIN
	impl().m_send_msg.set_capacity(capacity);
#endif
}
void TcpConnector::Impl::release()
{
	/*@ clear buffer when:
	1.tcp server close and destroy tcp peer:
	clear buffer by (TcpPeer.release in TcpAccepter)
	2.tcp client close tcp peer: (fix bugs)
	clear buffer by (TcpPeer.close -> TcpConnector.close)
	*/
#ifndef ECO_WIN
	m_send_msg.reset();
#endif
	m_buffer.clear();
}
void TcpConnector::release()
{
	impl().release();
}

////////////////////////////////////////////////////////////////////////////////
eco::String TcpConnector::ip() const
{
	return impl().get_ip();
}
uint32_t TcpConnector::port() const
{
	return impl().get_port();
}
bool TcpConnector::stopped() const
{
	return impl().stopped();
}


////////////////////////////////////////////////////////////////////////////////
}}