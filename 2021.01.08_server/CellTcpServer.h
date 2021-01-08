#pragma once
#ifndef _CELL_TCP_SERVER_
#define _CELL_TCP_SERVER_

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
	#include <map>
#else
#endif


#include "Message.h"
#include "CELLTimeStamp.h"
#include "INetEvent.h"
#define BUF_SIZE 1024

using namespace std;
class CellTCPServer
{
public:
	CellTCPServer(SOCKET sock) {
		ser_sock = sock;
		t1 = NULL;
		pEvent = NULL;
		fd_is_change = true;
	}

	~CellTCPServer() {

	}

	void SetEvent(INetEvent* event)
	{
		pEvent = event;
	}

	void Close() {
		for (auto elem : g_clients) {
			if (elem.second) delete elem.second;
			elem.second = NULL;
		}
		if (t1 != NULL)
		{
			delete t1;
			t1 = NULL;
		}
	}


	int RecvData(SocketMsgBuf* cnt_sock) {
		char buf[BUF_SIZE];
		int recv_len = recv(cnt_sock->sock, buf, BUF_SIZE, 0);
		if (recv_len <= 0) {
			closesocket(cnt_sock->sock);
			return -1;
		}
		/*DataHeader* header = (DataHeader*)buf;
		OnNextMsg(cnt_sock, header);*/
		recv_counts++;
		memcpy(cnt_sock->buf + cnt_sock->last_pos, buf, recv_len);
		cnt_sock->last_pos += recv_len;
		while (cnt_sock->last_pos >= sizeof(DataHeader)) {
			DataHeader* header = (DataHeader*)(cnt_sock->buf);
			if (cnt_sock->last_pos >= header->DataLength) {
				int nSize = cnt_sock->last_pos - header->DataLength;
				OnNextMsg(cnt_sock, header);
				memcpy(cnt_sock->buf, cnt_sock->buf + header->DataLength, nSize);
				cnt_sock->last_pos = nSize;
			}
			else break;
		}
		return 1;
	}

	int SendData(SocketMsgBuf* cnt_sock, DataHeader* header) {
		if (IsRun() && cnt_sock->sock != INVALID_SOCKET && header) {
			return send(cnt_sock->sock, (const char*)header, header->DataLength, 0);
		}
		return SOCKET_ERROR;
	}

	void OnNextMsg(SocketMsgBuf* cnt_sock, DataHeader* header) {
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			//LOGIN login;
			LOGINRESULT login_result;
			LOGIN* login = (LOGIN*)header;
			//cout << "收到的命令： CMD_LOGIN  数据长度： " << login->DataLength << " 用户姓名： " << login->UserName << " 密码： " << login->PassWord << endl;
			//忽略判断用户密码是否正确的过程
			login_result.result = 1;
			header = (DataHeader*)&login_result;
			//SendData(cnt_sock, header);
			//send(cnt_sock, (char*)&header, sizeof(DataHeader), 0);
		}
		break;
		case CMD_LOGOUT:
		{
			//LOGOUT logout;
			LOGOUTRESULT logout_result;
			LOGOUT* logout = (LOGOUT*)header;
			//cout << "收到的命令： CMD_LOGOUT  数据长度： " << logout->DataLength << " 用户姓名： " << logout->UserName << endl;
			//忽略判断用户密码的是否正确的过程
			logout_result.result = 1;
			header = (DataHeader*)&logout_result;
			//SendData(cnt_sock, header);
			//send(cnt_sock, (char*)&header, sizeof(DataHeader), 0);
		}
		break;
		default:
		{
			DataHeader header = { sizeof(DataHeader), CMD_ERROR };
		}
		break;
		}
	}

	int OnRun() {
		fd_set fdRead_cp;
		while (IsRun()) {
			//伯克利 socket 描述符
			fd_set fdRead;
			FD_ZERO(&fdRead);
			if (g_clients_buf_queue.size() > 0)
			{
				for (auto elem : g_clients_buf_queue)
				{
					g_clients[elem->sock] = elem;
				}
				g_clients_buf_queue.clear();
				fd_is_change = true;
			}

			if (g_clients.empty())
			{
				std::chrono::milliseconds t(10);
				std::this_thread::sleep_for(t);
				continue;
			}

			if (fd_is_change)
			{
				for (auto elem : g_clients)
				{
					FD_SET(elem.first, &fdRead);
				}
				fdRead_cp = fdRead;
				fd_is_change = false;
			}
			else
			{
				fdRead = fdRead_cp;
			}

			//nfds 是一个整数值  是指结合fd set集合中所有描述符（socket）的范围，而不是数量
			//即是所有描述符最大值 + 1，在windows中这个参数可以写0
			timeval t = { 0, 10 };
			int ret = select(ser_sock + 1, &fdRead, nullptr, nullptr, &t);
			if (ret < 0) {
				cout << "客户端已退出，任务结束！" << endl;
				return -1;
			}
#ifdef  _WIN32
			for (int i = 0; i < fdRead.fd_count; i++)
			{
				if (-1 == RecvData(g_clients[fdRead.fd_array[i]]))
				{
					pEvent->OnLeave(g_clients[fdRead.fd_array[i]]);
					pEvent->PrintLeaveSocket(g_clients[fdRead.fd_array[i]]);
					closesocket(fdRead.fd_array[i]);
					g_clients.erase(fdRead.fd_array[i]);
					fd_is_change = true;
				}

			}
#else
			for (auto elem : g_clients) {
				if (FD_ISSET(elem.first, &fdRead)) {
					if (-1 == RecvData(elem.second)) {
						pEvent->OnLeave(elem.second);
						closesocket(elem.first->sock);
						g_clients.erase(elem.first);
						fd_is_change = true;
					}
				}
		}
#endif //  _WIN32

		}
		return 1;
	}

	void push_client(SocketMsgBuf* sock)
	{
		g_clients_buf_queue.push_back(sock);
	}

	void Start()
	{
		t1 = new std::thread(&CellTCPServer::OnRun, this);
	}

	int Client_count()
	{
		return g_clients.size() + g_clients_buf_queue.size();
	}

	static int Process_recv_counts()
	{
		/*int counts = CellTCPServer::recv_counts;
		return counts;*/
		return CellTCPServer::recv_counts;
	}

	static void Reset_recv_counts()
	{
		CellTCPServer::recv_counts = 0;
	}

	bool IsRun() {
		return ser_sock != INVALID_SOCKET;
	}
private:
	SOCKET ser_sock;
	std::map<SOCKET, SocketMsgBuf*> g_clients;
	vector<SocketMsgBuf*> g_clients_buf_queue;
	std::mutex mutex;
	INetEvent* pEvent;
	std::thread* t1;
	bool fd_is_change;
	static std::atomic_int recv_counts;
};

std::atomic_int CellTCPServer::recv_counts = 0;

#endif // !_CELL_TCP_SERVER_
