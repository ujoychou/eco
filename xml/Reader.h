#ifndef ECO_XML_READER_H
#define ECO_XML_READER_H
/*******************************************************************************
@ name
xml key value reader.

@ function
1.read leaf value. format: <node>xx<node/>
2.read attribute "value=" value. format: <node value="xx">;

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
namespace xml{;
////////////////////////////////////////////////////////////////////////////////
class ECO_API Reader
{
	ECO_SHARED_API(Reader);
public:
	// read key/values from xml file by file path
	void read(
		OUT eco::Context& vals,
		IN  const char* file_path);
	
	// read xml nodes from xml file by file path
	void read(
		OUT eco::ContextNode& node,
		IN  const char* file_path);

	// read from xml text string.
	void read(
		OUT eco::ContextNode& node,
		IN  const char* xml_text,
		IN  const uint32_t xml_size);
};


////////////////////////////////////////////////////////////////////////////////
/*@ get sub path.*/
inline void sub_path(OUT std::string& sup, IN const char* sub)
{
	if (!sup.empty())
		sup += '/';
	sup += sub;
}


////////////////////////////////////////////////////////////////////////////////
}}
#endif