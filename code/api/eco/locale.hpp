#pragma once
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2024-09-04] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <eco/macro.hpp>


eco_namespace(eco);
////////////////////////////////////////////////////////////////////////////////
class locale
{
public:
    // set locale.
	void init(
		const char* locale_directory);

	// set locale default language.
	void set_default_language(
		const char* lang);

public:
	// set locale default language info.
	void set_language_info(
		const char* lang,
		const char* path,
		const char* version);

	// add dictionary error file.
	void add_error_file(
		const char* lang,
		const char* file,
		const char* module);

	// add dictionary word file.
	void add_word_file(
		const char* lang,
		const char* file,
		const char* module);

public:
    const char* get_error(
        int id,
        const char* module);
    
    const char* get_error(
        const char* id,
        const char* module);
};


#define eco_throw(...) eco::error(...)
////////////////////////////////////////////////////////////////////////////////
eco_namespace_end(eco);