#ifndef ECO_RX_HEAP_H
#define ECO_RX_HEAP_H
/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [history ver 1.0]
@ ujoy modifyed on 2016-05-06.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2017, ujoy, reserved all right.

*******************************************************************************/
#include <eco/Prec.h>
#include <eco/rx/RxApi.h>


#undef new
#undef delete
ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
class ECO_API RxHeap 
{
public:
	// new operators.
	static void* operator new(size_t size);

	static void* operator new[](size_t size);

	// debug new: indicate the filename and file line.
	static void* operator new(
		size_t size, const char* file_name, int file_line);

	static void* operator new[](
		size_t size, const char* file_name, int file_line);

	// delete operators.
	static void operator delete(void* p);

	static void operator delete[](void* p);

	// Unicode: leaving file_name as char for now
	static void operator delete(
		void* p, const char* file_name, int file_line);

	static void operator delete[](
		void* p, const char* file_name, int file_line);
};


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);
#endif