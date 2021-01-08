#pragma once
#ifndef EASY_TCP_CLIENT_H
#define EASY_TCP_CLIENT_H


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
#endif // ! _WIN32
#include <iostream>
#include "Message.h"
#define BUF_SIZE 1024
#define FD_SETSIZE      1000
using namespace std;


class EasyTCPClient
{
public:
	EasyTCPClient() {
		//
		cnt_sock = INVALID_SOCKET;
	}

	virtual ~EasyTCPClient() {

	}

	//初始化socket
	int initsocket() {
		//启动window sock 2.x环境
		if (INVALID_SOCKET != cnt_sock) {
			cout << "关闭了之前的连接！" << endl;
			Close();
		}
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
		cnt_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == cnt_sock) {
			std::cout << "错误， 建立socket失败！" << std::endl;
		}
		return 1;
	}

	//连接服务器
	int Connect(char* ip, unsigned short port) {
		if (INVALID_SOCKET == cnt_sock) {
			initsocket();
		}
		SOCKADDR_IN ser_add;
		ser_add.sin_addr.s_addr = inet_addr(ip);
		ser_add.sin_family = AF_INET;
		ser_add.sin_port = htons(port);
		int ret;
		if ((ret = connect(cnt_sock, (SOCKADDR*)&ser_add, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)) {
			cout << "错误，连接服务器失败！" << endl;
		}
		_sock = new SocketMsgBuf(cnt_sock);
		return ret;
	}

	//关闭socket
	void Close() {
		//关闭window sock 2.x环境
		if (INVALID_SOCKET == cnt_sock) {
			closesocket(cnt_sock);
			WSACleanup();
		}
		if (_sock != NULL) {
			delete _sock;
			_sock = NULL;
		}
		cnt_sock = INVALID_SOCKET;
	}

	//发送数据
	//接收数据
	//处理网络信息

	int SendData(DataHeader* header) {
		if (IsRun() && header) {
			return send(cnt_sock, (const char*)header, header->DataLength, 0);
		}
		return SOCKET_ERROR;
	}

	//接收数据  粘包问题 拆包  查询消息
	int RecvData() {
		char buf[1024];
		int rec_len = recv(cnt_sock, buf, 1024, 0);
		memcpy(_sock->buf + _sock->last_pos, buf, rec_len);
		_sock->last_pos += rec_len;
		if (rec_len <= 0) {
			cout << "与服务器断开连接！" << endl;
			return -1;
		}
		while (_sock->last_pos >= sizeof(DataHeader)) {
			DataHeader* header = (DataHeader*)(_sock->buf);
			if (_sock->last_pos >= header->DataLength) {
				int nSize = _sock->last_pos - header->DataLength;
				memcpy(_sock->buf, _sock->buf + header->DataLength, nSize);
				_sock->last_pos = nSize;
				OnNextMsg(header);
			}
			else break;
		}
		//cout << "收到的命令： " << header.cmd << " 数据长度： " << header.DataLength << endl;
		return 1;
	}

	bool OnRun() {
		while (IsRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(cnt_sock, &fdReads);
			timeval t = { 1, 0 };
			int ret = select(cnt_sock + 1, &fdReads, 0, 0, &t);
			if (ret < 0) {
				cout << "select任务结束！" << endl;
				return false;
			}
			if (FD_ISSET(cnt_sock, &fdReads)) {
				FD_CLR(cnt_sock, &fdReads);
				if (-1 == RecvData()) {
					cout << "客户端结束！" << endl;
					return false;
				}
			}
		}
		return true;
	}

	//相应消息
	void OnNextMsg(DataHeader* header) {
		switch (header->cmd)
		{
		case CMD_NEW_USER_JOIN:
		{
			NEW_USER_JOIN* new_user = (NEW_USER_JOIN*)header;
			cout << "收到服务端消息：CMD_NEW_USER_JOIN, 数据长度： " << new_user->DataLength << " 套接字： " << new_user->sock << endl;
		}
		break;
		case CMD_LOGIN_RESULT:
		{
			LOGINRESULT* login_ret = (LOGINRESULT*)header;
			cout << "收到服务端消息： CMD_LOGIN_RESULT， 数据长度： " << login_ret->DataLength << " 回复结果： " << login_ret->result << endl;
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LOGOUTRESULT* logout_ret = (LOGOUTRESULT*)header;
			cout << "收到服务端消息： CMD_LOGOUT_RESULT， 数据长度： " << logout_ret->DataLength << " 回复结果： " << logout_ret->result << endl;
		}
		break;
		default:
		{
			DataHeader header = { sizeof(DataHeader), CMD_ERROR };
			send(cnt_sock, (char*)&header, sizeof(DataHeader), 0);
		}
		break;
		}
	}

	bool IsRun() {
		return cnt_sock != INVALID_SOCKET;
	}
private:
	SOCKET cnt_sock;
	SocketMsgBuf* _sock;
};



#endif // !EASY_TCP_CLIENT
