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
#include <eco/ExportApi.h>
#include <eco/Typex.h>
#include <string>



namespace eco{;
namespace xml{;


////////////////////////////////////////////////////////////////////////////////
class ECO_API Reader
{
	ECO_SHARED_API(Reader);
public:
	void read(OUT eco::Context& vals, IN const char* file);
	void read(OUT eco::ContextNode& node, IN const char* file);
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