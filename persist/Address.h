#ifndef ECO_PERSIST_ADDRESS_H
#define ECO_PERSIST_ADDRESS_H
/*******************************************************************************
@ name
   persist address, using identify the database.

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2018-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2018 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>


namespace eco{;
namespace persist{;

// datasouce type.
enum 
{
	source_mysql		= 1,
	source_sqlite		= 2,
};
typedef uint32_t SourceType;


////////////////////////////////////////////////////////////////////////////////
class ECO_API Address
{
	ECO_VALUE_API(Address);
public:
	// address: "127.0.0.1:3306".
	Address& set(IN const char* addr);

	// data source type.
	void set_type(const SourceType);
	const SourceType get_type() const;
	SourceType type();
	Address& type(const SourceType);
	void set_type(const char*);

	// host name
	void set_name(const char*);
	const char* get_name() const;
	Address& name(const char*);

	// host ip
	void set_host(const char*);
	const char* get_host() const;
	Address& host(const char*);

	// port
	void set_port(const uint32_t);
	uint32_t port();
	const uint32_t get_port() const;
	Address& port(const uint32_t);

	// database
	void set_database(const char*);
	const char* get_database() const;
	Address& database(const char*);

	// user
	void set_user(const char*);
	const char* get_user() const;
	Address& user(const char*);

	// password
	void set_password(const char*);
	const char* get_password() const;
	Address& password(const char*);
};


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
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif