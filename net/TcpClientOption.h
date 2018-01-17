#ifndef ECO_NET_TCP_CLIENT_OPTION_H
#define ECO_NET_TCP_CLIENT_OPTION_H
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


namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpClientOption
{
	ECO_VALUE_API(TcpClientOption);
////////////////////////////////////////////////////////////////////////////////
public:
	// service name.
	void set_service_name(IN const char*);
	const char* get_service_name() const;
	TcpClientOption& service_name(IN const char*);

	// tick counter.
	void step_tick(IN const uint32_t step = 1);
	uint32_t tick_count() const;
	void reset_tick();

	/*@ set server tick time which is mininum unit time of server timer.
	* @ para.seconds: tick time seconds, if "=0" there will not be a tick timer.
	*/
	void set_tick_time(IN const uint32_t seconds);
	uint32_t tick_time();
	const uint32_t get_tick_time() const;
	TcpClientOption& tick_time(IN const uint32_t);
	bool has_tick_timer() const;

	/*@ set tcp server heartbeat ticks that include send tick and recv tick.
	send tick is control the rhythm of send heartbeat, and recv tick is used
	to judge peer of connection is still alive.
	* @ heartbeat_send_tick: how many ticks to server send heartbeat intervals.
	* @ heartbeat_recv_tick: if server doesn't recv a heartbeat from peer in recv
	ticks, it will close connection with the peer.
	*/
	void set_heartbeat_send_tick(IN const uint32_t);
	uint32_t heartbeat_send_tick();
	const uint32_t get_heartbeat_send_tick() const;
	TcpClientOption& heartbeat_send_tick(IN const uint32_t);

	void set_heartbeat_recv_tick(IN const uint32_t);
	uint32_t heartbeat_recv_tick();
	const uint32_t get_heartbeat_recv_tick() const;
	TcpClientOption& heartbeat_recv_tick(IN const uint32_t);

	/* the time auto reconnect to server.*/
	void set_auto_reconnect_tick(IN const uint32_t);
	uint32_t auto_reconnect_tick();
	const uint32_t get_auto_reconnect_tick() const;
	TcpClientOption& auto_reconnect_tick(IN const uint32_t);

	/* @ set tcp connection option whether has delay on sending data, and delay
	is to improve workload performace.
	*/
	void set_no_delay(IN const bool);
	bool no_delay() const;
	TcpClientOption& no_delay(IN const bool);

	/* @ set tcp connection option whether it is a websocket protocol.
	*/
	void set_websocket(IN const bool);
	bool websocket() const;
	TcpClientOption& websocket(IN const bool);
};

////////////////////////////////////////////////////////////////////////////////
}}
#endif