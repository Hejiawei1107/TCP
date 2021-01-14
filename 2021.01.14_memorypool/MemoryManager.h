#pragma once
#ifndef _MEMORYMANAGER_

#ifdef  _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include <Windows.h>
	#include <WinSock2.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <unistd.h> // uni std
	#include <arpa/inet.h>
	#include <string>

	#define SOCKET int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define SOCKET_ERROR			(-1)
#endif //  _WIN32

#include <stdio.h>
#include <stdlib.h>
#include <mutex>

const int MAX_COUNT =  128;

#ifdef _DEBUG
	#define Xprint(...) printf(__VA_ARGS__)
#else
	#define Xprint(...) 
#endif // DEBUG

class MemPool;

class MemoryBlock
{
public:
	MemoryBlock()
	{
		Uid = 0;
		Is_Ref = false;
		Is_Pool = false;
		From = NULL;
		Next = NULL;
	};

	~MemoryBlock() {};
	size_t Uid;
	size_t size;
	bool Is_Ref;
	bool Is_Pool;
	MemPool* From;
	MemoryBlock* Next;
	
};

class MemPool
{
public:
	MemPool()
	{
		MemoryBlock* header = NULL;
		size_t BlockSize = 0;
		size_t BlockNum = 0;
		char* buf = 0;
		Xprint("MemPool....\n");
	};

	~MemPool()
	{
		if (buf)
		{
			::free(buf);
			buf = NULL;
		}
	};

	void init()
	{
		if (buf)
		{
			return;
		}
		size_t size = BlockSize + sizeof(MemoryBlock);
		size_t Sum_size = size * BlockNum;
		buf = (char*)malloc(Sum_size);
		header = (MemoryBlock*)buf;
		header->Uid = 0;
		header->size = BlockSize;
		header->Is_Pool = true;
		header->Is_Ref = false;
		header->Next = NULL;

		MemoryBlock* tmp = header;

		for (int i = 1; i < BlockNum; i++)
		{
			MemoryBlock* block = (MemoryBlock*)(buf + size * i);
			block->Uid = i;
			block->size = BlockSize;
			block->Is_Ref = false;
			block->Is_Pool = true;
			block->From = this;
			block->Next = NULL;
			tmp->Next = block;
			tmp = block;
		}
	}

	void* alloc(size_t size)
	{
		if (header == NULL)
		{
			MemoryBlock* block = (MemoryBlock*)malloc(size + sizeof(MemoryBlock));
			block->Uid = -1;
			block->size = size;
			block->Is_Ref = true;
			block->Is_Pool = false;
			block->From = NULL;
			block->Next = NULL;
			//Xprint("New    Uid = %d, Size = %d, Pos = %p, IsPool = %d\n", block->Uid, block->size, block, block->Is_Pool);
			return (char*)block + sizeof(MemoryBlock);
		}
		else
		{
			if (header == NULL) return alloc(size);
			std::lock_guard<std::mutex> lock(_mutex);
			MemoryBlock* block = header;
			block->Is_Ref = true;
			header = header->Next;
			//Xprint("New    Uid = %d, Size = %d, Pos = %p, IsPool = %d\n", block->Uid, block->size, block, block->Is_Pool);
			return (char*)block + sizeof(MemoryBlock);
		}
	}

	void free(void* ptr)
	{
		MemoryBlock* block = (MemoryBlock*)((char*)ptr - sizeof(MemoryBlock));
		//Xprint("delete    Uid = %d, Size = %d, Pos = %p, IsPool = %d\n", block->Uid, block->size, block, block->Is_Pool);
		if (block->Is_Pool)
		{
			if (!block->Is_Ref) return;
			std::lock_guard<std::mutex> lock(_mutex);
			block->Is_Ref = false;
			block->Next = header;
			header = block;
		}
		else
		{
			::free(block);
		}
	}

public:
	MemoryBlock* header;
	size_t BlockSize;
	size_t BlockNum;
	char* buf;
	std::mutex _mutex;
};

template<size_t size, size_t num>
class MemoryPool : public MemPool
{
public:
	MemoryPool()
	{
		BlockSize = size;
		BlockNum = num;
		init();
	}
};

class MemoryManager
{
public:
	MemoryManager()
	{
		init_arr(0, 64, &_mem64);
		init_arr(65, 128, &_mem128);
		Xprint("MemoryManager....\n");
	};


	~MemoryManager() {};

	static MemoryManager& Istance()
	{
		static MemoryManager MemMan;
		return MemMan;
	}

	void init_arr(size_t start, size_t end, MemPool* mem)
	{
		for (size_t i = start; i <= end; i++)
		{
			arr[i] = mem;
		}
	}

	void* alloc(size_t size)
	{
		if (size <= MAX_COUNT)
		{
			return arr[size]->alloc(size);
		}
		else
		{
			MemoryBlock* block = (MemoryBlock*)malloc(size + sizeof(MemoryBlock));
			block->Uid = -1;
			block->Is_Pool = false;
			block->Is_Ref = true;
			block->size = size;
			block->Next = NULL;
			return (char*)block + sizeof(MemoryBlock);
		}
	}

	void free(void* ptr)
	{
		MemoryBlock* block = (MemoryBlock*)((char*)ptr - sizeof(MemoryBlock));
		if (block->size > MAX_COUNT)
		{
			::free(block);
		}
		else
		{
			arr[block->size]->free(ptr);
		}
	}

private:
	MemPool* arr[MAX_COUNT + 1];
	MemoryPool<64, 1000000> _mem64;
	MemoryPool<128, 101000000> _mem128;
};
#endif // !_MEMORYMANAGER_
