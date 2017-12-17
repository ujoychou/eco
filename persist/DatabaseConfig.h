#ifndef ECO_DATABASE_CONFIG_H
#define ECO_DATABASE_CONFIG_H
/*******************************************************************************
@ name

@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2017-07-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2017 - 2019, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Export.h>


namespace eco{;
////////////////////////////////////////////////////////////////////////////////
class DatabaseConfig
{
public:
	// get database relative field type expression sql.
	virtual void get_field_type_sql(
		OUT std::string& field_sql,
		IN  const uint32_t field_type,
		IN  const uint32_t field_size) = 0;
};


////////////////////////////////////////////////////////////////////////////////
}// ns::eco
#endif