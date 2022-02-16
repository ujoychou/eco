#include "Pch.h"
#include <eco/rx/RxHeap.h>
////////////////////////////////////////////////////////////////////////////////


ECO_NS_BEGIN(eco);
////////////////////////////////////////////////////////////////////////////////
void* RxHeap::operator new(size_t size)
{
	return ::operator new(size);
}
void* RxHeap::operator new[](size_t size)
{
	return ::operator new[](size);
}
void* RxHeap::operator new(size_t size, const char* file_name, int file_line)
{
	return ::operator new(size);
}
void* RxHeap::operator new[](size_t size, const char* file_name, int file_line)
{
	return ::operator new[](size);
}


////////////////////////////////////////////////////////////////////////////////
void RxHeap::operator delete(void* p) 
{   
	if (p != NULL) { ::operator delete(p); }
}
void RxHeap::operator delete[](void* p) 
{   
	if (p != NULL) { ::operator delete[](p); }
}
void RxHeap::operator delete(void* p, const char* file_name, int file_line)
{
	if (p != NULL) { ::operator delete(p); }
}
void RxHeap::operator delete[](void* p, const char* file_name, int file_line)
{
	if (p != NULL) { ::operator delete[](p); }
}


////////////////////////////////////////////////////////////////////////////////
ECO_NS_END(eco);