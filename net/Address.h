#ifndef ECO_NET_ADDRESS_H
#define ECO_NET_ADDRESS_H
/*******************************************************************************
@ name
   internet address, using identify the communication endpoint.

@ function
   suport 2 format address:
   1.ip address: "ip:port". exp:127.0.0.1:8080
   2.server address: "host_name:server_name". exp:localhost:datetime.

@ exception

@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>



namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
enum  
{
	service_mode		= 1,		// C\S架构服务
	router_mode			= 2,		// 路由架构服务
};
typedef uint16_t ServiceMode;


////////////////////////////////////////////////////////////////////////////////
class ECO_API Address
{
	ECO_VALUE_API(Address);
public:
	Address(IN const char* addr);

	/*@ set net address. clear the address info if @addr == nullptr.
	* @ para.addr_str: net address string.
	"ip:port": 127.0.0.1:80" "host_name:server_name": zhouyu:datetime"
	*/
	void set(IN const char* addr = nullptr);

	/*@ set net address.
	* @ para.ip: net address string. exp:"127.0.0.1"
	* @ para.port: net address port number. exp:"80"
	*/
	void set(
		IN const char* ip,
		IN const uint32_t port);

	// address name or alias that easy to remember.
	void set_name(const char*);
	const char* get_name() const;
	Address& name(const char*);

	// host name
	void set_host_name(const char*);
	const char* get_host_name() const;
	Address& host_name(const char*);

	// service name
	void set_service_name(const char*);
	const char* get_service_name() const;
	Address& service_name(const char*);

	// get port of ip address.
	const uint32_t get_port() const;

	// check the address is a ip format or hostname format.
	bool ip_format() const;

	// check is a empty address.
	bool empty() const;

	// check equal.
	inline bool operator==(IN const Address& addr) const
	{
		return equal(*this, addr);
	}
	inline static bool equal(IN const Address& addr1, IN const Address& addr2)
	{
		return strcmp(addr1.get_host_name(), addr2.get_host_name()) == 0
			&& strcmp(addr1.get_service_name(), addr2.get_service_name()) == 0;
	}
};
template<typename Stream>
Stream& operator<<(OUT Stream& stream, IN const Address& v)
{
	return stream < v.get_host_name() < ':' < v.get_service_name()
		< eco::group(v.get_name());
}


////////////////////////////////////////////////////////////////////////////////
// thread safe.
class ECO_API AddressSet
{
	ECO_SHARED_API(AddressSet);
public:
	typedef eco::iterator<Address> iterator;
	typedef eco::iterator<const Address> const_iterator;
	iterator begin();
	const_iterator begin() const;
	iterator end();
	const_iterator end() const;

	/*@ add address.*/
	void add(IN const Address&);
	Address& add();
	void push_back(IN const Address&);

	/*@ move & copy address.*/
	void add_move(IN AddressSet&);
	void add_copy(IN const AddressSet&);
	
	/*@ remove address.*/
	void erase(IN int);
	iterator erase(IN iterator& it);
	void pop_back();

	/*@ clear address.*/
	void clear();

	/*@ get address set size.*/
	size_t size() const;
	bool empty() const;
	void reserve(IN const size_t capacity);

	/*@ access address by item index.*/
	Address& at(IN const int i);
	const Address& at(IN const int i) const;

public:
	// module name or alias that easy to remember.
	void set_name(const char*);
	const char* get_name() const;
	AddressSet& name(const char*);

	// service mode or router mode.
	const ServiceMode get_mode() const;
	void set_mode(IN const ServiceMode);
	ServiceMode mode();
	AddressSet& mode(IN const ServiceMode);
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif