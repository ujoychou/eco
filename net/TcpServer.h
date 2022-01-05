#ifndef ECO_NET_TCP_SERVER_H
#define ECO_NET_TCP_SERVER_H
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
#include <eco/net/TcpOption.h>
#include <eco/net/TcpDispatch.h>
#include <eco/net/TcpConnection.h>
#include <eco/net/protocol/Protocol.h>


ECO_NS_BEGIN(eco);
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpServer : public TcpDispatch
{
	ECO_SHARED_API(TcpServer);
public:
	/*@ start tcp server.*/
	void start();

	/*@ stop this server, and wait all the request in server to be handled.*/
	void stop();

	/*@ join this server threads.*/
	void join();

	// network server option.
	void set_option(IN const TcpServerOption&);
	TcpServerOption& get_option();
	const TcpServerOption& option() const;
	TcpServer& option(IN const TcpServerOption&);

	// protocol
	template<typename protocol_t>
	inline void add_protocol()
	{
		add_protocol(new protocol_t());
	}
	void add_protocol(IN Protocol*);

	// get latest protocol.
	Protocol* protocol_latest() const;
	Protocol* protocol(int ver) const;

	// set connection data type.
	template<typename connection_data_t>
	inline void set_connection_data()
	{
		set_connection_data(&make_connection_data<connection_data_t>);
	}
	void set_connection_data(IN MakeConnectionData&& make);

	// set session data class and tcp session mode.
	template<typename session_data_t>
	inline void set_session_data()
	{
		set_session_data(&make_session_data<session_data_t>);
	}
	void set_session_data(IN MakeSessionData&& make);

	// make session id.
	uint64_t make_object_id(uint32_t& ts, uint32_t& seq, int ver = 1);

	// register dispatch handler.
	virtual void register_default(IN HandlerFunc&& hf) override;
	virtual void register_handler(IN int id, IN HandlerFunc&& hf) override;

public:
	// set connection open close event
	void set_event(OnAccept&&, OnClose&&);

	// set receive callback event.
	void set_recv_event(OnRecvData&&, OnDecodeHead&&);

	// is receive mode: using recv callback event to handle message.
	bool receive_mode() const;

	// is dispatch mode: using eco dispatch mechanism to handle message.
	bool dispatch_mode() const;
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif