#ifndef ECO_NET_ASIO_TCP_CONNECTOR_H
#define ECO_NET_ASIO_TCP_CONNECTOR_H
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
#include <eco/net/Net.h>




////////////////////////////////////////////////////////////////////////////////
namespace eco{;
namespace net{;


class TcpSocket;
class IoService;
class ProtocolHead;
////////////////////////////////////////////////////////////////////////////////
class TcpConnectorHandler
{
public:
	virtual void on_connect(
		IN bool is_connected,
		IN const eco::Error* error)
	{}

	virtual void on_read_head(
		IN char* data,
		IN const uint32_t head_size,
		IN const eco::Error* error) = 0;

	virtual void on_read_data(
		IN eco::String& data,
		IN const eco::Error* error) = 0;

	virtual void on_write(
		IN const uint32_t write_size,
		IN const eco::Error* error) = 0;
};


////////////////////////////////////////////////////////////////////////////////
class TcpPeer;
class TcpConnector : eco::Object<TcpConnector>
{
	ECO_IMPL_API();
public:
	TcpConnector(IN IoService* srv);
	~TcpConnector();

	// register handler and handler life control.
	void register_handler(
		IN TcpConnectorHandler& handler,
		IN std::weak_ptr<TcpPeer>& peer);

	// get tcp connection id.
	ConnectionId get_id() const;

	// get remote ip address.
	const eco::String get_ip() const;

	// get tcp connection socket.
	TcpSocket* socket();

	// set tcp connector option.
	void set_option(IN bool delay);

	/*@ client async connect to server.	*/
	void async_connect(IN const Address& addr);

	/*@ asynchronous read data head from client.
	* @ para.data: memory space for storing comming data.
	* @ para.size: size of memory space.
	*/
	void async_read_head(IN char* data, IN const uint32_t head_size);

	/*@ asynchronous read data from client.
	* @ para.data: memory space for storing comming data.
	*/
	void async_read_data(IN eco::String& data, IN const uint32_t start);

	/*@ async read data until meet the "delimiter" string.
	* @ para.size: memory space for storing comming data.
	*/
	void async_read_until(
		IN const uint32_t data_size,
		IN const char* delimiter);

	/*@ asynchronous send data to client.
	* @ para.data: data to send to client.
	*/
	void async_write(IN eco::String& data, IN const uint32_t start);

	// close socket.
	void close();
};


}}
////////////////////////////////////////////////////////////////////////////////
#endif