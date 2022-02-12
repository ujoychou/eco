#ifndef ECO_RECORD_SHEET_H
#define ECO_RECORD_SHEET_H
/*******************************************************************************
@ name


@ function


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2013-01-01.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2013 - 2015, ujoy, reserved all right.

*******************************************************************************/
#include <eco/rx/RxApi.h>


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class ECO_API Record
{
	ECO_OBJECT_API(Record);
public:
	// reserve data size.
	void reserve(IN size_t size);

	// get data size.
	size_t size() const;

	// add data.
	void add(IN const char* item);

	// get data set value by index. 
	const char* at(IN size_t index) const;

	// get data set value by index. 
	const char* operator[](IN size_t index) const;

	// clear data.
	void clear();
};


////////////////////////////////////////////////////////////////////////////////
class ECO_API Recordset
{
	ECO_OBJECT_API(Recordset);
public:
	// reserve data size.
	void reserve(IN size_t size);

	// get data size.
	size_t size() const;

	// add data.
	Record& add_item();

	// get dataset of the datasheet.
	Record& at(IN size_t index);
	const Record& at(IN size_t index) const;

	// get dataset of the datasheet.
	Record& operator[](IN size_t index);
	const Record& operator[](IN size_t index) const;

	// clear data.
	void clear();
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif