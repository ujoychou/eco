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
#include <eco/net/DispatchRegistry.h>
#include <eco/net/SessionData.h>
#include <eco/net/TcpServerOption.h>
#include <eco/net/protocol/Protocol.h>


namespace eco{;
namespace net{;
////////////////////////////////////////////////////////////////////////////////
class ECO_API TcpServer
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
	TcpServerOption& option();
	const TcpServerOption& get_option() const;
	TcpServer& option(IN const TcpServerOption&);

	/*@ set protocol head.*/
	template<typename ProtocolHeadT>
	inline void set_protocol_head()
	{
		set_protocol_head(new ProtocolHeadT());
	}
	void set_protocol_head(IN ProtocolHead* heap);
	ProtocolHead& protocol_head() const;

	/*@ register protocol.*/
	template<typename ProtocolT>
	inline void register_protocol()
	{
		register_protocol(new ProtocolT);
	}
	void register_protocol(IN Protocol*);

	// set session data class and tcp session mode.
	template<typename SessionDataT>
	inline void set_session_mode()
	{
		set_session_mode(&make_session_data<SessionDataT>);
	}
	/*@ set tcp server session mode.
	* @ make: validate session factory function.
	*/
	void set_session_mode(IN MakeSessionDataFunc make);

	// dispatcher
	DispatchRegistry& dispatcher();
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif