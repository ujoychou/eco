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
	char_set_gbk		= 1,
	char_set_gb2312		= 2,
	char_set_utf8		= 3,
	char_set_utf16		= 4,
	char_set_utf32		= 5,
};
typedef uint16_t CharSet;

////////////////////////////////////////////////////////////////////////////////
class ECO_API Address
{
	ECO_VALUE_API(Address);
public:
	// address: "127.0.0.1:3306".
	Address& set(IN const char* addr);

	// data source type.
	void set_type(IN const SourceType);
	const SourceType get_type() const;
	SourceType type();
	Address& type(IN const SourceType);
	// type string: mysql/sqlite/.
	void set_type(IN const char* type);
	const char* get_type_name() const;

	// data source char set.
	void set_char_set(IN const CharSet = char_set_gbk);
	const CharSet get_char_set() const;
	CharSet char_set();
	Address& char_set(IN const CharSet = char_set_gbk);

	// host name
	void set_name(IN const char*);
	const char* get_name() const;
	Address& name(IN const char*);

	// host ip
	void set_host(IN const char*);
	const char* get_host() const;
	Address& host(IN const char*);

	// port
	void set_port(IN const uint32_t);
	uint32_t port();
	const uint32_t get_port() const;
	Address& port(IN const uint32_t);

	// database
	void set_database(IN const char*);
	const char* get_database() const;
	Address& database(IN const char*);

	// user
	void set_user(IN const char*);
	const char* get_user() const;
	Address& user(IN const char*);

	// password
	void set_password(IN const char*);
	const char* get_password() const;
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