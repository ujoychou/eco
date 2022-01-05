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


ECO_NS_BEGIN(eco);
namespace persist{;

// data souce type.
enum 
{
	source_mysql		= 1,
	source_sqlite		= 2,
};
typedef uint16_t SourceType;


// data source client character set.
enum
{
	charset_gbk			= 1,
	charset_gb2312		= 2,
	charset_utf8		= 3,
	charset_utf16		= 4,
	charset_utf32		= 5,
};
typedef uint16_t Charset;

////////////////////////////////////////////////////////////////////////////////
class ECO_API Address
{
	ECO_VALUE_API(Address);
public:
	// address: "127.0.0.1:3306".
	Address& set_address(IN const char* addr);

	// data source type.
	void set_type(IN const SourceType);
	const SourceType type() const;
	SourceType& get_type();
	Address& type(IN const SourceType);
	// type string: mysql/sqlite/.
	void set_type(IN const char* type);
	const char* type_name() const;

	// data source char set.
	void set_charset(IN const Charset = charset_gbk);
	const Charset charset() const;
	Charset& get_charset();
	Address& charset(IN const Charset = charset_gbk);
	void set_charset(IN const char* v = "gbk");

	// host name
	void set_name(IN const char*);
	const char* name() const;
	Address& name(IN const char*);

	// host ip
	void set_host(IN const char*);
	const char* host() const;
	Address& host(IN const char*);

	// port
	void set_port(IN const uint32_t);
	uint32_t& get_port();
	const uint32_t port() const;
	Address& port(IN const uint32_t);

	// database
	void set_database(IN const char*);
	const char* database() const;
	Address& database(IN const char*);

	// user
	void set_user(IN const char*);
	const char* user() const;
	Address& user(IN const char*);

	// password
	void set_password(IN const char*);
	const char* password() const;
	Address& password(IN const char*);
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