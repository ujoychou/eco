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
#include <eco/Typex.h>



namespace eco{;
////////////////////////////////////////////////////////////////////////////////
class ECO_API Config
{
	ECO_OBJECT_API(Config);
public:
	/*@ read config data from file.*/
	void init(IN const std::string& file);

	/*@ find key return true, else false.*/
	bool find(
		OUT eco::StringAny& v,
		IN const char* key) const;

	/*@ get value.*/
	const eco::StringAny at(
		IN const char* key) const;

	/*@ add key and value.*/
	void add(
		IN const char* key,
		IN const char* value);

	/*@ get children node set.*/
	void get_children(
		OUT eco::ContextNodeSet& node_set,
		IN const char* parent_key) const;

	/*@ get node property.*/
	void get_property_set(
		OUT eco::Context& context_set,
		IN  const char* node_key) const;

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