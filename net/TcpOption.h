#ifndef ECO_NET_TCP_OPTION_H
#define ECO_NET_TCP_OPTION_H
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
#include <eco/ExportApi.h>


ECO_NS_BEGIN(eco);
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpOption
{
	ECO_VALUE_API(TcpOption);
public:
	// service name or client name.
	void set_name(IN const char*);
	const char* name() const;
	TcpOption& name(IN const char*);

	// router name.
	void set_router(IN const char*);
	const char* router() const;
	TcpOption& router(IN const char*);

	/*@ io send_buffer_size.*/
	void set_max_byte_size(IN const uint32_t);
	uint32_t& get_max_byte_size();
	const uint32_t max_byte_size() const;
	TcpOption& max_byte_size(IN const uint32_t);

	/*@ set tcp heartbeat ticks that include send tick and recv tick.
	send tick is control the rhythm of send heartbeat, and recv tick is used
	to judge peer of connection is still alive.
	* @ heartbeat_send_sec: how many ticks to send heartbeat intervals.
	* @ heartbeat_recv_sec: if peer doesn't recv a heartbeat from remote in
	recv ticks, it will close connection with the peer.
	*/
	void set_heartbeat_send_sec(IN const uint32_t);
	uint32_t& get_heartbeat_send_sec();
	const uint32_t heartbeat_send_sec() const;
	TcpOption& heartbeat_send_sec(IN const uint32_t);

	void set_heartbeat_recv_sec(IN const uint32_t);
	uint32_t& get_heartbeat_recv_sec();
	const uint32_t heartbeat_recv_sec() const;
	TcpOption& heartbeat_recv_sec(IN const uint32_t);

	/*@ set tcp server/client heartbeat mode.
	* @ para.io_heartbeat: if "true", each connection will set a timer by itself
	to send	heartbeat, so the thread of sending heartbeat is same with thread
	that handle	connection request, but it will reduce the performance of
	server.
	and there is another mode, when set this parameter "false", heartbeat
	transactions will be handled by a global timer thread that all connection
	shared.
	* @ para.rhythm_heartbeat mode: if "true", server send heartbeat in regular
	intervals, but sometimes it's not needed, the purpose of heartbeat is judge
	a live peer, when connection send a message also indicated it still alive.
	so if "false", means when connection has produce a live action, there will
	be no need to send heartbeat, and heartbeat will not be send rhythmly.
	* @ para.response_heartbeat mode: if "true", server will response a
	heartbeat when it receive a heartbeat from peer, else it just send heartbeat
	independently, we suggest the latter one.
	*/
	void set_heartbeat_rhythm(IN const bool);
	bool heartbeat_rhythm() const;
	TcpOption& heartbeat_rhythm(IN const bool);

	/*@ request context capacity.*/
	void set_context_capacity(IN const uint32_t);
	uint32_t& get_context_capacity();
	const uint32_t context_capacity() const;
	TcpOption& context_capacity(IN const uint32_t);

	/*@ io send capacity.*/
	void set_send_capacity(IN const uint32_t);
	uint32_t& get_send_capacity();
	const uint32_t send_capacity() const;
	TcpOption& send_capacity(IN const uint32_t);

	/*@ io send_buffer_size.*/
	void set_send_buffer_size(IN const uint32_t);
	uint32_t& get_send_buffer_size();
	const uint32_t send_buffer_size() const;
	TcpOption& send_buffer_size(IN const uint32_t);

	/*@ io recv_buffer_size.*/
	void set_recv_buffer_size(IN const uint32_t);
	uint32_t& get_recv_buffer_size();
	const uint32_t recv_buffer_size() const;
	TcpOption& recv_buffer_size(IN const uint32_t);

	/*@ io receive_low_watermark.*/
	void set_recv_low_watermark(IN const uint32_t);
	uint32_t& get_recv_low_watermark();
	const uint32_t recv_low_watermark() const;
	TcpOption& recv_low_watermark(IN const uint32_t);

	/*@ io receive_low_watermark.*/
	void set_send_low_watermark(IN const uint32_t);
	uint32_t& get_send_low_watermark();
	const uint32_t send_low_watermark() const;
	TcpOption& send_low_watermark(IN const uint32_t);

	/* @ set tcp connection option whether has delay on sending data, and delay
	is to improve workload performace.
	*/
	void set_no_delay(IN const bool);
	bool no_delay() const;
	TcpOption& no_delay(IN const bool);

	/* @ set tcp connection option whether it is a websocket protocol.
	*/
	void set_websocket(IN const bool);
	bool websocket() const;
	TcpOption& websocket(IN const bool);
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpClientOption : public TcpOption
{
	ECO_VALUE_API(TcpClientOption, TcpOption);
public:
	enum
	{
		balance_average		= 1,
		balance_order		= 2,
	};

	// client module name.
	void set_module_(IN const char*);
	const char* module_() const;
	TcpClientOption& module_(IN const char*);

	/* the time auto reconnect to server.*/
	void set_auto_reconnect_sec(IN const uint16_t);
	uint16_t& get_auto_reconnect_sec();
	const uint16_t auto_reconnect_sec() const;
	TcpClientOption& auto_reconnect_sec(IN const uint16_t);

	/*@ load_evnets timeout.*/
	void set_load_event_sec(IN const uint16_t);
	uint16_t& get_load_event_sec();
	const uint16_t load_event_sec() const;
	TcpClientOption& load_event_sec(IN const uint16_t);

	/*@ connect server with balance algorithm.*/
	void set_balance(IN const uint16_t);
	uint16_t& get_balance();
	const uint16_t balance() const;
	TcpClientOption& balance(IN const uint16_t);
	void set_balance(IN const char*);

	// whether client suspend.
	void set_suspend(IN const bool);
	bool suspend() const;
	TcpClientOption& suspend(IN const bool);
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpServerOption : public TcpOption
{
	ECO_VALUE_API(TcpServerOption, TcpOption);
public:
	// server horizontal virtual service number.
	const uint32_t horizontal_virtual_service_number() const;

	// server port.
	void set_port(IN const uint32_t);
	uint32_t& get_port();
	const uint32_t port() const;
	TcpServerOption& port(IN const uint32_t);

	// max connection size this server can accept.
	void set_max_connection_size(IN const uint32_t);
	uint32_t& get_max_connection_size();
	const uint32_t max_connection_size() const;
	TcpServerOption& max_connection_size(IN const uint32_t);

	// max session size this server can contain.
	void set_max_session_size(IN const uint32_t);
	uint32_t& get_max_session_size();
	const uint32_t max_session_size() const;
	TcpServerOption& max_session_size(IN const uint32_t);

	/*@ set clean dos peer tick time.
	*/
	void set_clean_dos_peer_sec(IN const uint32_t);
	uint32_t& get_clean_dos_peer_sec();
	const uint32_t clean_dos_peer_sec() const;
	TcpServerOption& clean_dos_peer_sec(IN const uint32_t);

	/*@ server io thread size to handle net data.*/
	void set_io_thread_size(IN const uint16_t);
	uint16_t& get_io_thread_size();
	const uint16_t io_thread_size() const;
	TcpServerOption& io_thread_size(IN const uint16_t);

	/*@ server business thread size to handle request.*/
	void set_business_thread_size(IN const uint16_t);
	uint16_t& get_business_thread_size();
	const uint16_t business_thread_size() const;
	TcpServerOption& business_thread_size(IN const uint16_t);

public:
	inline uint32_t	horizental_number() const
	{
		return port() / horizontal_virtual_service_number();
	}
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif