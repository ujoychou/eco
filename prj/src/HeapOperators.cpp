#include "PrecHeader.h"
#include <eco/HeapOperators.h>
////////////////////////////////////////////////////////////////////////////////




namespace eco{;


////////////////////////////////////////////////////////////////////////////////
void* HeapOperators::operator new(size_t size)
{
	return ::operator new(size);
}

void* HeapOperators::operator new[](size_t size)
{
	return ::operator new[](size);
}

// Unicode: leaving file_name as char for now
void* HeapOperators::operator new(
	size_t size,
	const char* file_name,
	int file_line)
{
	return ::operator new(size);
}

void* HeapOperators::operator new[](
	size_t size,
	const char* file_name,
	int file_line)
{
	return ::operator new[](size);
}

void HeapOperators::operator delete(void* p) 
{   
	if (p != NULL){
		::operator delete(p);
	}
}

void HeapOperators::operator delete[](void* p) 
{   
	if (p != NULL){
		::operator delete[](p);
	}
}

// Unicode: leaving file_name as char for now
void HeapOperators::operator delete(
	void* p, const char* file_name,	int file_line)
{
	if (p != NULL){
		::operator delete(p);
	}
}

void HeapOperators::operator delete[](
	void* p, const char* file_name, int file_line)
{
	if (p != NULL){
		::operator delete[](p);
	}
}


////////////////////////////////////////////////////////////////////////////////
}