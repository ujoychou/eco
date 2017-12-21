#ifndef ECO_NET_TCP_CLIENT_IPP
#define ECO_NET_TCP_CLIENT_IPP
#include "PrecHeader.h"
#include <eco/net/TcpClient.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include <eco/thread/State.h>
#include <eco/Repository.h>
#include <eco/net/Worker.h>
#include <eco/net/IoTimer.h>
#include <eco/net/TcpClientOption.h>
#include <eco/net/DispatchServer.h>
#include <eco/net/protocol/TcpProtocol.h>
#include "TcpPeer.ipp"
#include "TcpSession.ipp"


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(net);
////////////////////////////////////////////////////////////////////////////////
class AddressLoad
{
public:
	inline AddressLoad(IN const Address& addr)
		: m_address(addr)
		, m_flag(0)
	{}

	inline bool operator==(IN const Address& addr) const
	{
		return (m_address == addr);
	}

	Address		m_address;
	uint16_t	m_workload;
	uint16_t    m_flag;
};


////////////////////////////////////////////////////////////////////////////////
class SessionDataPack
{
	ECO_OBJECT(SessionDataPack);
public:
	eco::String m_request;
	SessionData::ptr m_session;
	SharedObserver m_user_observer;
	uint32_t m_auto_login;

public:
	inline SessionDataPack(IN bool auto_login = false)
		: m_auto_login(auto_login)
	{}
};


////////////////////////////////////////////////////////////////////////////////
class TcpClient::Impl : public TcpPeerHandler
{
public:
	TcpClient*					m_client;
	std::string					m_service_name;
	TcpClientOption				m_option;
	Protocol::ptr				m_protocol;
	std::auto_ptr<ProtocolHead> m_prot_head;
	eco::atomic::State			m_state;
	eco::Mutex m_mutex;

	// network io.
	TcpPeer::ptr			m_peer;
	AddressLoad*			m_address_cur;
	std::vector<AddressLoad> m_address_set;
	eco::net::Worker		m_worker;
	eco::net::IoTimer		m_timer;
	// message server.
	DispatchServer			m_dispatcher;
	
	// session management.
	MakeSessionDataFunc m_make_session;
	eco::Repository<void*, SessionDataPack::ptr> m_authority_map;
	eco::Repository<uint32_t, SessionDataPack::ptr> m_session_map;
	std::vector<eco::net::TcpSession> m_session_wait;
	eco::Mutex m_session_wait_mutex;

public:
	inline Impl() : m_client(nullptr)
	{}

	inline void init(IN TcpClient& v)
	{
		m_client = &v;
		m_address_cur = nullptr;
		m_peer = TcpPeer::make((IoService*)m_worker.get_io_service(), this);
	}

	// register protocol.
	inline void set_protocol_head(IN ProtocolHead* v)
	{
		m_prot_head.reset(v);
	}
	inline void set_protocol(IN Protocol* p)
	{
		m_protocol.reset(p);
	}

public:
	// waited session.
	inline void get_session_wait(
		IN std::vector<eco::net::TcpSession>& sess_set)
	{
		eco::Mutex::ScopeLock lock(m_session_wait_mutex);
		sess_set = m_session_wait;
		m_session_wait.clear();
	}
	inline void add_session_wait(IN eco::net::TcpSession& sess)
	{
		eco::Mutex::ScopeLock lock(m_session_wait_mutex);
		m_session_wait.push_back(sess);
		m_session_wait.clear();
	}

	// find exist session.
	inline SessionDataPack::ptr find_session(IN const SessionId id) const
	{
		SessionDataPack::ptr v;
		m_session_map.find(v, id);
		return v;
	}

	// erase session.
	inline void erase_session(IN const SessionId id)
	{
		SessionDataPack::ptr v = m_session_map.pop(id);
		if (v != nullptr)
		{
			eco::thread::release(v);
		}
	}

public:
	// build connection.
	inline void async_connect(
		IN const char* service_name_,
		IN eco::net::AddressSet& service_addr);
	inline void async_connect();

	/*@ start tick timer that do some auxiliary.*/
	inline void set_tick_timer();
	void on_timer(IN const eco::Error* e);

public:
	// when peer has connected to server.
	virtual void on_connect() override;

	// when peer has received a message data bytes.
	virtual void on_read(
		IN void* peer,
		IN eco::String& data) override;

	// protocol.
	virtual ProtocolHead& protocol_head() const override
	{
		return *m_prot_head;
	}
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(net);
ECO_NS_END(eco);
#endif