#ifndef ECO_WEB_HTML_H
#define ECO_WEB_HTML_H
/*******************************************************************************
@ name
singleton model solution.

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2015-01-15.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/Typex.h>


namespace eco{;
namespace web{;
////////////////////////////////////////////////////////////////////////////////
/* support nested html class define.
1.class format: ' class="$(class_name)"', and class name can be empty.
2.class define is supoort nested.
3.filter annotation: <!-- class="$(xxx)"-->.
4.match mode suport: <xx / > and <xx ></xx >.
5.class start pos is the first blank pos that before '<'.

reference tpl file: "main.html".
*/
class ECO_API HtmlClass
{
	ECO_SHARED_API(HtmlClass);
public:
	// get parent position.
	const char* GetName() const;

	// get html class content.
	const char* GetContent() const;

	// get position in parent context.
	size_t GetPosition() const;

	// add child html class.
	HtmlClass AddChild();

	// get child class.
	HtmlClass FindChild(
		IN const char* class_name);

	// compare the child class pos defined in html.
	int Compare(
		IN const HtmlClass& child_class_1,
		IN const HtmlClass& child_class_2);
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API HtmlObject
{
	ECO_SHARED_API(HtmlObject);
public:
	enum 
	{
		class_pos = 1,
		order_pos,
	};
	typedef int PositionMode;

	// set property value.
	void SetValue(
		IN const char* prop_name,
		IN const char* value);

	// add child html object.
	HtmlObject AddChild(
		IN const char* class_name,
		IN PositionMode pos_mode = class_pos);

	// get html class.
	HtmlClass GetClass() const;

	// get the element pos mode.
	PositionMode GetPositionMode() const;

	// make html text.
	void MakeText(OUT eco::StringAny& text) const;
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Html
{
	ECO_SHARED_API(Html);
public:
	// load html file.
	static Html LoadFile(IN const char* file);

	// add html element
	HtmlObject Root();

	// save html
	void Save();

	// save html to indicated path.
	void SaveAs(IN const char* file);
};


////////////////////////////////////////////////////////////////////////////////
}}// ns
#endif