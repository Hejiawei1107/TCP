// easyTCPclient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include "easyTCPclient.h"


using namespace std;

char* ip;
short port;
int per_thread_client_counts;
bool is_on = true;
int Pthread_counts = 4;
int client_counts = 1000;
std::mutex m;
//void cmdThread(EasyTCPClient* client) {
//    while (true) {
//        char cmdBuf[256];
//        cin >> cmdBuf;
//        if (!strcmp(cmdBuf, "exit")) {
//            cout << "客户端退出！" << endl;
//            client->Close();
//            break;
//        }
//        else if (!strcmp(cmdBuf, "login")) {
//            LOGIN login;
//            strcpy_s(login.UserName, "何家为");
//            strcpy_s(login.PassWord, "hejiawei");
//            client->SendData(&login);
//        }
//        else if (!strcmp(cmdBuf, "logout")) {
//            LOGOUT logout;
//            strcpy_s(logout.UserName, "何家为");
//            client->SendData(&logout);
//        }
//        else {
//            cout << "不支持的命令！" << endl;
//        }
//    }
//}


void cmdexit() {
    while (true) {
        char buf[128];
        cin >> buf;
        if (!strcmp(buf, "exit")) {
            is_on = false;
            return;
        }
    }
}


void pthread(int id) {
    vector<EasyTCPClient*> clients;
    int start = (id - 1) * per_thread_client_counts, end = min(id * per_thread_client_counts, client_counts);
    for (int i = start; i < end; i++) {
        clients.push_back(new EasyTCPClient());
        std::lock_guard<std::mutex> lock(m);
        cout << "thread <" << id << ">  connect <" << i << ">"<< endl;
    }
    for (int i = 0; i < clients.size(); i++) {
        clients[i]->initsocket();
        clients[i]->Connect(ip, port);
    }
    std::chrono::milliseconds t(100);
    std::this_thread::sleep_for(t);
    LOGIN login;
    strcpy_s(login.UserName, "何家为");
    strcpy_s(login.PassWord, "hejiawei");
    while (is_on) {
        for (int i = 0; i < clients.size(); i++) {
            if (!is_on) {
                break;
            }
            clients[i]->SendData(&login);
        }
    }
    for (int i = 0; i < clients.size(); i++) {
        clients[i]->Close();
        delete clients[i];
        clients[i] = NULL;
    }
}


int main(int argc, char* argv[])
{
    ip = argv[1];
    port = stoi(argv[2]);
    per_thread_client_counts = client_counts / Pthread_counts;
    std::thread t1(cmdexit);
    t1.detach();
    for (int i = 1; i <= Pthread_counts; i++)
    {
        std::thread t(pthread, i);
        t.detach();
    }
    while (is_on)
    {

    }
    return 0;



}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
