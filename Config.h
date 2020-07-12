#ifndef ECO_CONFIG_H
#define ECO_CONFIG_H
/*******************************************************************************
@ name
key value config data.

@ function
1.support read from config xml.
2.supoort read from config ini.

@ exception

@ note

--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2018, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Type.h>


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
class ECO_API Config
{
	ECO_SHARED_API(Config);
public:
	/*@ read config data from file.*/
	void init(IN const char* file);

	/*@ read config data from xml text.*/
	void init(IN const char* text, IN const uint32_t size);

	/*@ get import file by index.*/
	uint32_t get_import_file_size() const;
	const char* get_import_file(IN uint32_t index) const;

	/*@ find key return true, else false.*/
	bool find(
		OUT eco::StringAny& v,
		IN const char* key) const;

	/*@ get value.*/
	const eco::StringAny at(
		IN const char* key) const;

	/*@ get value.*/
	const eco::StringAny get(
		IN const char* key) const;

	/*@ add key and value.*/
	void add(
		IN const char* key,
		IN const char* value);

	/*@ get children node set.*/
	eco::ContextNodeSet get_children(
		IN const char* parent_key = nullptr) const;
	eco::ContextNodeSet find_children(
		IN const char* parent_key = nullptr) const;

	/*@ get child node.*/
	eco::ContextNode get_node(
		IN const char* node_key = nullptr) const;
	eco::ContextNode find_node(
		IN const char* node_key = nullptr) const;

	/*@ get child node.*/
	eco::ContextNode find_node(
		IN const uint32_t index,
		IN const char* parent_key = nullptr) const;

	/*@ get node property.*/
	void get_property_set(
		OUT eco::Context& context_set,
		IN  const char* node_key = nullptr) const;

	/*@ has key return true, else false.*/
	inline bool has(IN const char* key) const
	{
		eco::StringAny v;
		return find(v, key);
	}
};


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif