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
#include <eco/net/Address.h>
#include <eco/net/Net.h>




////////////////////////////////////////////////////////////////////////////////
ECO_NS_BEGIN(eco);
namespace net{;
class TcpSocket;
class IoWorker;
class MessageHead;
////////////////////////////////////////////////////////////////////////////////
class TcpConnectorHandler
{
public:
	virtual void on_connect(bool err) {}

	virtual void on_read(MessageHead& head, eco::String& data, bool err) = 0;

	virtual void on_write(uint32_t siz, bool err) = 0;
};


////////////////////////////////////////////////////////////////////////////////
class TcpPeer;
class TcpConnector : eco::Object<TcpConnector>
{
	ECO_IMPL_API();
public:
	TcpConnector(IN IoWorker* srv);
	~TcpConnector();

	// register handler and handler life control.
	void register_handler(
		IN TcpConnectorHandler& handler,
		IN std::weak_ptr<TcpPeer>& peer);

	// get remote ip address.
	eco::String ip() const;
	uint32_t port() const;

	// io worker stopped.
	bool stopped() const;

	// get tcp connection socket.
	TcpSocket* socket();

	// set no delay option for nagle.
	void set_no_delay(IN bool delay);
	bool no_delay() const;

	// set send cache buffer size.
	void set_send_buffer_size(IN int size);
	int  get_send_buffer_size() const;

	// set recv cache buffer size.
	void set_recv_buffer_size(IN int size);
	int  get_recv_buffer_size() const;

	// set send cache buffer size.
	void set_send_low_watermark(IN int size);
	int  get_send_low_watermark() const;

	// set recv cache buffer size.
	void set_recv_low_watermark(IN int size);
	int  get_recv_low_watermark() const;

	/*@ client async connect to server.	*/
	bool async_connect(IN const Address& addr);

	/*@ asynchronous read data head from client.
	* @ para.data: memory space for storing comming data.
	* @ para.size: size of memory space.
	*/
	void async_read();

	/*@ asynchronous read data from client.
	* @ para.data: memory space for storing comming data.
	*/
	void async_read_data(IN eco::String& data, IN const uint32_t start);

	/*@ async read data until meet the "delimiter" string.
	* @ para.size: memory space for storing comming data.
	*/
	void async_read_until(IN const char* delimiter);

	/*@ asynchronous send data to client.
	* @ para.data: data to send to client.
	*/
	void async_write(IN eco::String& data, IN const uint32_t start);

	/*@ sync send data to client.
	* @ para.data: data to send to client.
	*/
	bool write(IN eco::String& data, IN const uint32_t start);

	/*@ set io send string list capacity.
	*/
	void set_send_capacity(IN int capacity);

	// reset connector data.
	void release();

	// close socket.
	void close();
};


}}
////////////////////////////////////////////////////////////////////////////////
#endif