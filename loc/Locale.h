#ifndef ECO_LOC_LOCALE_H
#define ECO_LOC_LOCALE_H
/*******************************************************************************
@ name
log server.

@ function

@ exception

@ note


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-05-09.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2015 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/ExportApi.h>
#include <eco/proto/Object.pb.h>


ECO_NS_BEGIN(eco);
ECO_NS_BEGIN(loc);
////////////////////////////////////////////////////////////////////////////////
class ECO_API Locale
{
	ECO_OBJECT_API(Locale);
public:
	// set locale.
	void add_locale(
		IN const eco::proto::Locale& loc,
		IN const char* mdl);

	// set locale language.
	void set_default(
		IN const char* lang);

	// set locale default language info.
	void set_language_info(
		IN const char* lang,
		IN const char* path,
		IN const char* ver);

	// add dictionary word file.
	void add_error_file(
		IN const char* lang,
		IN const char* file,
		IN const char* mdl);

	// add dictionary word file.
	void add_word_file(
		IN const char* lang,
		IN const char* file,
		IN const char* mdl);

public:
	// get locale language proto data.
	const char* language() const;

	// get error message by error id.
	const char* get_error(
		IN int eid,
		IN const char* lang = "");
	eco::Format<>& get_error_format(
		IN int eid,
		IN const char* lang = "");

	// get error message by error id and module.
	// @when module api callback.
	const char* get_error(
		IN int eid,
		IN const char* para,
		IN const char* mdl,
		IN const char* lang = "");

	// get error message by string path.
	// @when module api callback.
	const char* get_error(
		IN const char* path,
		IN const char* para = "",
		IN const char* mdl = "",
		IN const char* lang = "");
	eco::Format<>& get_error_format(
		IN const char* path,
		IN const char* lang = "");

	// get word view.
	const char* get_word(
		IN const char* path,
		IN const char* lang = "");
	eco::Format<>& get_word_format(
		IN const char* path,
		IN const char* lang = "");

	// get variable view.
	// var: "{var1}{var2}{var3}"
	const char* get_variable(
		IN const char* var,
		IN const char* mdl = "",
		IN const char* lang = "");
	eco::Format<>& get_variable_format(
		IN const char* var,
		IN const char* mdl = "",
		IN const char* lang = "");

	// find locale node.
	bool find(
		OUT eco::StringAny& result,
		IN  const char* key,
		IN  const char* lang) const;

	// get locale node.
	eco::StringAny get(
		IN  const char* key,
		IN  const char* lang) const;

	// get locale file path of language.
	eco::StringAny get_path(
		IN  const char* key,
		IN  const char* lang) const;

	// get locale proto data.
	const eco::proto::Locale& data() const;

	// get locale language proto data.
	const proto::Language* language(IN const char* lang = "") const;
};


ECO_API Locale& locale();
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(loc);
ECO_NS_END(eco);
#endif