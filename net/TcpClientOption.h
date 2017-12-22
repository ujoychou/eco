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
	void set_name(IN const char*);
	const char* get_name() const;
	TcpClientOption& name(IN const char*);

	// tick counter.
	void step_tick(IN const uint32_t step = 1);
	uint32_t tick_count() const;
	void reset_tick();

	/*@ set server tick time which is mininum unit time of server timer.
	* @ para.seconds: tick time seconds, if "=0" there will not be a tick timer.
	*/
	void set_tick_time(IN const uint32_t& seconds);
	uint32_t& tick_time();
	const uint32_t& get_tick_time() const;
	TcpClientOption& tick_time(IN const uint32_t&);
	bool has_tick_timer() const;

	/*@ set tcp server heartbeat ticks that include send tick and recv tick.
	send tick is control the rhythm of send heartbeat, and recv tick is used
	to judge peer of connection is still alive.
	* @ heartbeat_send_tick: how many ticks to server send heartbeat intervals.
	* @ heartbeat_recv_tick: if server doesn't recv a heartbeat from peer in recv
	ticks, it will close connection with the peer.
	*/
	void set_heartbeat_send_tick(IN const uint32_t&);
	uint32_t& heartbeat_send_tick();
	const uint32_t& get_heartbeat_send_tick() const;
	TcpClientOption& heartbeat_send_tick(IN const uint32_t&);

	void set_heartbeat_recv_tick(IN const uint32_t&);
	uint32_t& heartbeat_recv_tick();
	const uint32_t& get_heartbeat_recv_tick() const;
	TcpClientOption& heartbeat_recv_tick(IN const uint32_t&);

	/*@ set tcp server heartbeat mode.
	* @ para.is_conn_mode: if "true", each connection will set a timer by itself
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
	void set_response_heartbeat(IN const bool);
	bool response_heartbeat() const;
	TcpClientOption& response_heartbeat(IN const bool);

	void set_rhythm_heartbeat(IN const bool);
	bool rhythm_heartbeat() const;
	TcpClientOption& rhythm_heartbeat(IN const bool);

	/*@ set intervals that clean the inactive connection peer.
	* @ para.tick: the server ticks of interval.
	*/
	void set_clean_inactive_peer_tick(IN const uint32_t& tick);
	uint32_t& clean_inactive_peer_tick();
	const uint32_t& get_clean_inactive_peer_tick() const;
	TcpClientOption& clean_inactive_peer_tick(IN const uint32_t&);

	/*@ server io thread size to handle net data.*/
	void set_io_thread_size(IN const uint16_t&);
	uint16_t& io_thread_size();
	const uint16_t& get_io_thread_size() const;
	TcpClientOption& io_thread_size(IN const uint16_t&);

	/*@ server business thread size to handle request.*/
	void set_business_thread_size(IN const uint16_t&);
	uint16_t& business_thread_size();
	const uint16_t& get_business_thread_size() const;
	TcpClientOption& business_thread_size(IN const uint16_t&);

	/* the time auto reconnect to server.*/
	void set_auto_reconnect_tick(IN const uint32_t&);
	uint32_t& auto_reconnect_tick();
	const uint32_t& get_auto_reconnect_tick() const;
	TcpClientOption& auto_reconnect_tick(IN const uint32_t&);

	/* @ set tcp connection option whether has delay on sending data, and delay
	is to improve workload performace.
	*/
	void set_no_delay(IN const bool);
	bool no_delay() const;
	TcpClientOption& no_delay(IN const bool);

	/* @ set tcp connection option whether has session.
	*/
	void set_session_mode(IN const bool);
	bool session_mode() const;
	TcpClientOption& session_mode(IN const bool);
};

////////////////////////////////////////////////////////////////////////////////
}}
#endif