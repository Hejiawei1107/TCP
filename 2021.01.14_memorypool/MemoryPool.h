#pragma once
#ifndef _MEMORYPOOL_

#include "MemoryManager.h"

void* operator new(size_t size)
{
	return MemoryManager::Istance().alloc(size);
}

void* operator new[](size_t size)
{
	return MemoryManager::Istance().alloc(size);
}

void operator delete(void* ptr)
{
	MemoryManager::Istance().free(ptr);
}

void operator delete[](void* ptr)
{
	MemoryManager::Istance().free(ptr);;
}

#endif // !_MEMORYPOOL_
