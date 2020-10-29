#include <stdio.h>  
#include <winsock2.h>  
#include <iostream>
#include <string.h>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
//RECV线程函数定义
DWORD WINAPI Client_recv(LPVOID ipParameter)
{
    int RET;
    char RecvBuffer[MAX_PATH] = { '\0 ' };
    char SendBuffer[MAX_PATH] = { '\0' };
    SOCKET CclientScoket = (SOCKET)ipParameter;
    if (CclientScoket == INVALID_SOCKET)
    {
        cout << "client error " << endl;
    }
    while (true) {
        RET = recv(CclientScoket, RecvBuffer, MAX_PATH, 0);
        if (RET == 0 || RET == SOCKET_ERROR)
        {
            cout << "服务器相应失败，退出" << endl;
            break;
        }
        printf(RecvBuffer);
        cout << endl;
        memset(RecvBuffer, 0x00, MAX_PATH);
    }
    return 0;
}
//主函数
int main(void)
{
    WSADATA     WSA;
    SOCKET      clientScoket;
    struct      sockaddr_in  serveAddr;
    int         AddrLen = 0;
    HANDLE      hThread_1 = NULL;
    int         RET = 0;
    char        SendBuffer[MAX_PATH];
    char        RecvBuffer[MAX_PATH];
    //init windows socket
    if (WSAStartup(MAKEWORD(2, 2), &WSA) != 0)
    {
        cout << "init failed" << endl;
        return -1;
    }
    //creat socket
    clientScoket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientScoket == INVALID_SOCKET)
    {
        cout << "creat failed" << GetLastError() << endl;
        return -1;
    }

    serveAddr.sin_family = AF_INET;
    serveAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveAddr.sin_port = htons(4000);
    memset(serveAddr.sin_zero, 0x00, 8);

    RET = connect(clientScoket, (struct sockaddr*)&serveAddr, sizeof(serveAddr));
    if (RET != 0)
    {
        cout << "connect failed";
        return -1;
    }
    else { cout << "链接成功" << endl; }
    hThread_1 = CreateThread(NULL, 0, Client_recv, (LPVOID)clientScoket, 0, NULL);
    if (hThread_1 == NULL)
    {
        cout << "creat thread_1 failed" << endl;
    }
    \
    while (true)
    {
        cin.getline(SendBuffer, sizeof(SendBuffer));
        RET = send(clientScoket, SendBuffer, (int)strlen(SendBuffer), 0);
        if (strcmp(SendBuffer, "quit") == 0)
        {
            return 0;
        }else if (RET == SOCKET_ERROR)
        {
            cout << "send to error" << endl;
        }
        
    }

    return 0;


}