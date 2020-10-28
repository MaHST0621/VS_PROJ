#include <stdio.h>  
#include <winsock2.h>  
#include <iostream>
#include <string.h>
#include <map>
#include <vector>
using namespace std;
#pragma comment(lib,"ws2_32.lib")  
map<string, SOCKET>list_socket;
pair<map<string, SOCKET>::iterator, bool> Insert_Pair;
map<string, SOCKET>::iterator iter;
#define MAX_BUFF 2040;
char RecvBuffer[MAX_PATH] = { '\0 ' };
char SendBuffer[MAX_PATH] = { '\0' };


//创建线程
string find_key(SOCKET s)
{
    for (iter = list_socket.begin(); iter != list_socket.end(); iter++)
    {
        if (iter->second == s)
            return iter->first;
    }

}

vector<string> find_SendId(string s)
{
    vector<string> result;
    string result_0("");
    string temp("");
    for (int i = 0; i < s.size(); i++)
    {

        if (s[i] == ':')
        {
            result_0 = temp;
            temp = "";
            break;

        }
        else
        {
            temp += s[i];
        }
    }
    result_0 = result_0 + ":";
    for (int i = 0; i <= result_0.size(); i++)
    {
        if (result_0[i] == '|' || result_0[i] == ':')
        {
            result.push_back(temp);
            temp = "";

        }
        else
        {
            temp += result_0[i];
        }
    }
    return result;
}




DWORD WINAPI ClientThread(LPVOID ipParameter)
{
    SOCKET ClientScoket = (SOCKET)ipParameter;
    int RET = 0;
    //初始化 recvBuffer
    while (true) {
        memset(RecvBuffer, 0x00, sizeof(RecvBuffer));
        RET = recv(ClientScoket, RecvBuffer, MAX_PATH, 0);
        if (RET == 0 || RET == SOCKET_ERROR)
        {
            cout << "failed,exit" << endl;
            break;
        }

        cout << find_key(ClientScoket) << " :  " << RecvBuffer << endl;
    }



    return 0;


}



/*void client_thread(SOCKET client_sock, SOCKADDR_IN b)
{
    int RET = 0;
    char recv_buff[1024];
    while (true)
    {
        memset(recv_buff, 0, sizeof(recv_buff));
        RET = recv(client_sock, recv_buff, 1024, 0);
        if (RET == 0 || RET == SOCKET_ERROR)
        {
            cout << "faild exit" << endl;
        }
    }
    cout << inet_ntoa(b.sin_addr) << "发送的消息为：" << recv_buff << endl;
}
*/
int main(void)
{
    WSADATA     WSA;
    SOCKET      severScoket, clientScoket;
    struct      sockaddr_in  LocalAddr, clientAddr;
    int         AddrLen = 0;
    HANDLE      hThread_1 = NULL;
    HANDLE      hThread_2 = NULL;
    int         RET = 0;
    //init windows socket
    if (WSAStartup(MAKEWORD(2, 2), &WSA) != 0)
    {
        cout << "init failed" << endl;
        return -1;
    }
    //creat socket
    severScoket = socket(AF_INET, SOCK_STREAM, 0);
    if (severScoket == INVALID_SOCKET)
    {
        cout << "creat failed" << GetLastError() << endl;
        return -1;
    }
    LocalAddr.sin_family = AF_INET;
    LocalAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    LocalAddr.sin_port = htons(4000);
    memset(LocalAddr.sin_zero, 0x00, 8);
    //bind socket
    RET = bind(severScoket, (SOCKADDR*)&LocalAddr, sizeof(LocalAddr));
    if (RET != 0)
    {
        cout << "bind failed";
        return -1;
    }
    RET = listen(severScoket, 5);
    if (RET != 0)
    {
        cout << "listen failed";
        return -1;
    }
    cout << "服务器已启动" << endl;
    while (true)
    {
        AddrLen = sizeof(clientAddr);
        clientScoket = accept(severScoket, (SOCKADDR*)&clientAddr, &AddrLen);
        if (clientScoket == INVALID_SOCKET)
        {
            cout << "accept failed";
            break;
        }
        else
        {
            recv(clientScoket, RecvBuffer, sizeof(RecvBuffer), 0);
            Insert_Pair = list_socket.insert(map<string, SOCKET>::value_type(RecvBuffer, clientScoket));
            if (Insert_Pair.second == true)
            {
                for (iter = list_socket.begin(); iter != list_socket.end(); iter++)
                {
                    strcpy(SendBuffer, "欢迎");
                    strcat(SendBuffer, RecvBuffer);
                    strcat(SendBuffer, "加入服务器");
                    send(iter->second, SendBuffer, sizeof(SendBuffer), 0);
                    cout << iter->first << " " << endl;
                }
                memset(SendBuffer, 0x00, MAX_PATH);
            }
            else
            {
                send(clientScoket, "你的用户名已被使用请重新链接,谢谢", 34, 0);
                closesocket(clientScoket);
            }
        }
        hThread_1 = CreateThread(NULL, 0, ClientThread, (LPVOID)clientScoket, 0, NULL);
        if (hThread_1 == NULL)
        {
            cout << "creat thread_1 failed" << endl;
        }
        CloseHandle(hThread_1);
    }
    closesocket(severScoket);
    closesocket(clientScoket);
    WSACleanup();
    return 0;
}