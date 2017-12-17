#ifndef ECO_WEB_JSON_H
#define ECO_WEB_JSON_H
/*******************************************************************************
@ name

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-01-15.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Typex.h>



namespace eco{;
namespace web{;
////////////////////////////////////////////////////////////////////////////////
/*nested json class.
1.the json tpl is a valid json format.
2.json array is a class, you can add object to the array.
3.json object property can be set values.
4.json value type support: string, double, int.

reference tpl file: "equity_series.tpl".
*/
class ECO_API JsonClass
{
	ECO_SHARED_API(JsonClass);
public:
	// get json class name.
	const char* GetName() const;

	// get json class content.
	const char* GetContent() const;

	// get parent position.
	size_t GetParentPosition() const;

	// add child json class.
	JsonClass AddChild();

	// find json object class.
	JsonClass FindChild(
		IN const char* class_name);
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API JsonObject
{
	ECO_SHARED_API(JsonObject);
public:
	// set object name
	void SetObjectName(
		IN const char* old_name,
		IN const char* new_name);

	// set object property value: double.
	void SetValue(
		IN const char* prop_name,
		IN const double v);

	// set object property value: int.
	void SetValue(
		IN const char* prop_name,
		IN const int v);

	// set object property value: string.
	void SetValue(
		IN const char* prop_name,
		IN const char* v);

	// add json object.
	JsonObject AddChild(
		IN const char* class_name);

	// get json class.
	JsonClass GetClass() const;

	// make html text.
	void MakeText(OUT eco::StringAny& text) const;
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Json
{
	ECO_SHARED_API(Json);
public:
	// load json file.
	static Json LoadFile(
		IN const char* file);

	// get json root node.
	JsonObject Root();

	// save json.
	void Save();

	// save json to a file path.
	void SaveAs(
		IN const char* file);
};


////////////////////////////////////////////////////////////////////////////////
}}// ns
#endif