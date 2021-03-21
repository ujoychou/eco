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
#include <eco/eco/Proto.h>


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
	void set_default_(
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
	// get default language.
	const char* default_() const;

	// get error message by error id.
	const char* get_error(
		IN int eid,
		IN const char* lang = "");
	eco::FormatX& get_error_format(
		IN int eid,
		IN const char* lang = "");

	// get error message by string path.
	eco::FormatX& get_error_format(
		IN const char* path,
		IN const char* lang = "");

	// get word view.
	const char* get_word(
		IN const char* path,
		IN const char* mdl = "",
		IN const char* lang = "");
	eco::FormatX& get_word_format(
		IN const char* path,
		IN const char* mdl = "",
		IN const char* lang = "");

	// get variable view.
	// var: "{var1}{var2}{var3}"
	const char* get_variable(
		IN const char* var,
		IN const char* mdl = "",
		IN const char* lang = "");
	eco::FormatX& get_variable_format(
		IN const char* var,
		IN const char* mdl = "",
		IN const char* lang = "");

	// find locale node value.
	bool find(
		OUT eco::StringAny& result,
		IN  const char* key,
		IN  const char* lang) const;

	// get locale node value.
	eco::StringAny get(
		IN  const char* key,
		IN  const char* lang) const;

	// get locale proto data.
	const eco::proto::Locale& data() const;

	// get locale language proto data.
	const proto::Language* language(
		IN const char* lang = "") const;

public:
	// front server: return parse error message, and return to product client.
	const char* parse_error(
		IN const char* path,
		IN const char* para,
		IN const char* mdl,
		IN const char* lang = "");
	const char* parse_error(
		IN int eid,
		IN const char* para,
		IN const char* mdl,
		IN const char* lang = "");

	// front server: parse error from background services.
	inline const char* parse_error(
		OUT eco::proto::Error& e,
		IN  const char* mdl,
		IN  const char* lang)
	{
		const char* msg = e.path().empty()
			? parse_error(e.id(), e.value().c_str(), mdl, lang)
			: parse_error(e.path().c_str(), e.value().c_str(), mdl, lang);
		if (!eco::empty(msg)) e.set_value(msg);
		return e.value().c_str();
	}

	// front server: parse error from background services.
	inline const char* parse_error(const char* mdl, const char* lang)
	{
		eco::proto::Error& e = eco::this_thread::proto::error();
		return parse_error(e, mdl, lang);
	}
};


eco::loc::Locale& app_locale();
////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(loc);
ECO_NS_END(eco);
#endif