#pragma once
#ifndef _EASY_TCP_SERVER_
#define _EASY_TCP_SERVER_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include <Windows.h>
	#include <WinSock2.h>
	#include <iostream>
	#include <string>
	#include <vector>
#else
#endif


#include "Message.h"
#include "CELLTimeStamp.h"
#include "CellTcpServer.h"
#include "INetEvent.h"

#define PTHREAD_COUNTS 4

using namespace std;
class EasyTCPServer : public INetEvent
{
public:
	EasyTCPServer() {
		ser_sock = INVALID_SOCKET;
		recv_counts = 0;
		client_counts = 0;
		fd_is_change = true;
	}

	~EasyTCPServer() {

	}

	int inintsock()
	{
		if (IsRun()) {
			Close();
		}
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
		if ((ser_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
			cout << "socket创建失败！" << endl;
			return -1;
		}
		return 1;
	}
	void Close() {
		for (int i = 0; i < g_cellservers.size(); i++) {
			if (g_cellservers[i] != NULL)
			{
				g_cellservers[i]->Close();
				delete g_cellservers[i];
				g_cellservers[i] = NULL;
			}
		}
		if (IsRun()) {
			closesocket(ser_sock);
			WSACleanup();
		}
		ser_sock = INVALID_SOCKET;
	}

	void Bind(unsigned short port) {
		SOCKADDR_IN ser_add;
		ser_add.sin_family = AF_INET;
		ser_add.sin_port = htons(port);
		ser_add.sin_addr.s_addr = INADDR_ANY;
		if (bind(ser_sock, (SOCKADDR*)&ser_add, sizeof(ser_add)) == SOCKET_ERROR) {
			cout << "bind() error!" << endl;
			Close();
		}
		else {
			cout << "bind() succeed!" << endl;
		}
	}

	void Listen() {
		if (listen(ser_sock, 5) == SOCKET_ERROR) {
			cout << "listen() error!" << endl;
			Close();
		}
		else {
			cout << "listen() succeed!" << endl;
		}
	}

	virtual void OnLeave(SocketMsgBuf* pClient)
	{
		for (int i = g_clients.size() - 1; i >= 0; i--)
		{
			if (g_clients[i] == pClient)
			{
				auto iter = g_clients.begin() + i;
				g_clients.erase(iter);
			}
		}
	}

	virtual void PrintLeaveSocket(SocketMsgBuf* pClient)
	{
		std::lock_guard<std::mutex> lock(m);
		cout << "客户端<" << pClient->sock << ">	离开"  << endl;
	}

	int Accept() {

		SOCKADDR_IN cnt_add;
		int cnt_sock_len = sizeof(cnt_add);
		SOCKET cnt_sock;
		if ((cnt_sock = accept(ser_sock, (SOCKADDR*)&cnt_add, &cnt_sock_len)) == INVALID_SOCKET) {
			cout << "错误， 接收到无效客户端socket.....!" << endl;
			return -1;
		}
		//for (int n = g_clients.size() - 1; n >= 0; n--) {
		//	NEW_USER_JOIN new_join;
		//	//SendData(g_clients[n], &new_join);
		//}
		client_counts++;
		Add_Clients_To_Cellserver(new SocketMsgBuf(cnt_sock));
		return 1;
	}

	int OnRun()
	{
		while (IsRun())
		{
			//伯克利 socket 描述符
			fd_set fdRead;

			FD_ZERO(&fdRead);
			FD_SET(ser_sock, &fdRead);
			timeval t = { 0, 10 };
			if (timer.getElapsedTimeSec() >= 1)
			{
				per_second_recv_counts();
			}
			int ret = select(ser_sock + 1, &fdRead, nullptr, nullptr, &t);
			if (ret < 0) {
				cout << "客户端已退出，任务结束！" << endl;
				return -1;
			}
			if (FD_ISSET(ser_sock, &fdRead))
			{
				Accept();
			}
		}
		return 1;
	}

	void Add_Clients_To_Cellserver(SocketMsgBuf* sock) {
		if (g_cellservers.empty()) return;
		CellTCPServer* server = g_cellservers[0];
		g_clients.push_back(sock);
		for(int i = 1; i < PTHREAD_COUNTS; i++)
		{
			if (server->Client_count() > g_cellservers[i]->Client_count())
			{
				server = g_cellservers[i];
			}
		}
		server->push_client(sock);
	}

	void load() {
		for (int i = 0; i < PTHREAD_COUNTS; i++)
		{
			g_cellservers.push_back(new CellTCPServer(ser_sock));
			g_cellservers[i]->Start();
			g_cellservers[i]->SetEvent(this);
		}
		timer.update();
	}

	void per_second_recv_counts() {
		cout << "<thread> " <<  PTHREAD_COUNTS  << " <socket> " << ser_sock << " <clients> "<< g_clients.size() << " <time> " << timer.getElapsedTimeSec() << " <per_recvc_counts> "<< CellTCPServer::Process_recv_counts() / timer.getElapsedTimeSec() << endl;
		CellTCPServer::Reset_recv_counts();
		timer.update();
	}
	

	bool IsRun() {
		return ser_sock != INVALID_SOCKET;
	}
private:
	SOCKET ser_sock;
	vector<CellTCPServer*> g_cellservers;
	vector<SocketMsgBuf*> g_clients;
	CELLTimestamp timer;
	std::mutex m;
	fd_set fdread_cp;
	bool fd_is_change;
	int recv_counts;
	int client_counts;
};


#endif // !_EASY_TCP_SERVER_
