#pragma once
#ifndef _INET_EVENT_
#define _INET_EVENT_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include <Windows.h>
	#include <WinSock2.h>
	#include <iostream>
	#include <string>
	#include <vector>
	#include <mutex>
	#include <atomic>
	#include <thread>
#else
#endif

#include "Message.h"

class INetEvent
{
public:
	INetEvent() {};
	~INetEvent() {};
	virtual void OnLeave(SocketMsgBuf* pClient) = 0;
	virtual void PrintLeaveSocket(SocketMsgBuf* pClient) = 0;
private:
};


#endif
