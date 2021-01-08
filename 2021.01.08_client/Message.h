#pragma once
#ifndef MESSAGE
#define MESSAGE

struct DataPackage
{
    int age;
    char name[32];
};

enum CMD
{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,
    CMD_ERROR
};

struct DataHeader
{
    short DataLength; // the length of data
    short cmd;
};

struct LOGIN :public DataHeader
{
    LOGIN() {
        DataLength = sizeof(LOGIN);
        cmd = CMD_LOGIN;
    }
    char UserName[32];
    char PassWord[32];
};

struct LOGINRESULT :public DataHeader
{
    LOGINRESULT() {
        DataLength = sizeof(LOGINRESULT);
        cmd = CMD_LOGIN_RESULT;
    }
    int result;
};

struct LOGOUT :public DataHeader
{
    LOGOUT() {
        DataLength = sizeof(LOGOUT);
        cmd = CMD_LOGOUT;
    }
    char UserName[32];
};

struct LOGOUTRESULT :public DataHeader
{
    LOGOUTRESULT() {
        DataLength = sizeof(LOGOUTRESULT);
        cmd = CMD_LOGOUT_RESULT;
    }
    int result;
};


struct NEW_USER_JOIN :public DataHeader
{
    NEW_USER_JOIN() {
        DataLength = sizeof(NEW_USER_JOIN);
        cmd = CMD_NEW_USER_JOIN;
        sock = 0;
    }
    int sock;
};

class SocketMsgBuf
{
public:

    SocketMsgBuf(int socket = INVALID_SOCKET) : sock(socket), last_pos(0) {
        memset(buf, 0, sizeof(buf));
    };
    ~SocketMsgBuf() {};
public:
    char buf[10240];
    int last_pos;
    int sock;
};
#endif // !MESSAGE
