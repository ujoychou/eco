#include "PrecHeader.h"
#include <eco/net/Net.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/detail/socket_ops.hpp>



////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace net{;

static std::string s_ip;
static std::string s_hostname;

////////////////////////////////////////////////////////////////////////////////
const char* get_hostname()
{
	if (!s_hostname.empty())
	{
		return s_hostname.c_str();
	}

	using namespace boost::asio::ip;
	const char* error_head = "get host name error: ";
	boost::system::error_code ec;
	s_hostname = boost::asio::ip::host_name(ec);
	if (ec)
	{
		throw std::logic_error((error_head + ec.message()).c_str());
	}
	return s_hostname.c_str();
}


////////////////////////////////////////////////////////////////////////////////
const char* get_ip()
{
	if (!s_ip.empty())
	{
		return s_ip.c_str();
	}

	const char* error_head = "get local ip error: ";
	// get ip from local host name.
	using namespace boost::asio::ip;
	boost::system::error_code ec;
	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(tcp::v4(), get_hostname(), "");
	tcp::resolver::iterator it = resolver.resolve(query, ec);
	if (ec)
	{
		throw std::logic_error((error_head + ec.message()).c_str());
	}
	s_ip = it->endpoint().address().to_string(ec);
	if (ec)
	{
		throw std::logic_error((error_head + ec.message()).c_str());
	}
	return s_ip.c_str();
}


////////////////////////////////////////////////////////////////////////////////
uint16_t ntoh16(IN const char* str)
{
	using namespace boost::asio::detail::socket_ops;
	uint16_t be16 = 0;
	::memcpy(&be16, str, sizeof(be16));
	return network_to_host_short(be16);
}
void hton(OUT char* str, IN  const uint16_t val)
{
	using namespace boost::asio::detail::socket_ops;
	uint16_t be16 = host_to_network_short(val);
	::memcpy(str, &be16, sizeof(be16));
}
uint32_t ntoh32(IN const char* str)
{
	using namespace boost::asio::detail::socket_ops;
	uint32_t be32 = 0;
	::memcpy(&be32, str, sizeof(be32));
	return network_to_host_long(be32);
}
void hton(OUT char* str, IN  const uint32_t val)
{
	using namespace boost::asio::detail::socket_ops;
	uint32_t be32 = host_to_network_long(val);
	::memcpy(str, &be32, sizeof(be32));
}


////////////////////////////////////////////////////////////////////////////////
}}