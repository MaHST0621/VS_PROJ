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
char RecvBuffer[MAX_PATH] = { '\0 ' };
char SendBuffer[MAX_PATH] = { '\0' };


//通过该SOCKET映射出该客户端的昵称
string find_key(SOCKET s)
{
    for (iter = list_socket.begin(); iter != list_socket.end(); iter++)
    {
        if (iter->second == s)
            return iter->first;
    }

}
//从RECVBUFF中提取接受对象昵称
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
//string的反转函数
string Reverse(string s, int n) {
    for (int i = 0, j = n - 1; i < j; i++, j--) {
        char c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
    return s;
}
//提取RECVBUFF中的消息部分
string cut_sendbuff(string s)
{

    string result_0("");
    string temp("");
    for (int i = s.size() - 1; i >= 0; i--)
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
    result_0 = Reverse(result_0, result_0.size());

    return result_0;
}
//client线程定义
DWORD WINAPI ClientThread(LPVOID ipParameter)
{
    SOCKET ClientScoket = (SOCKET)ipParameter;
    int RET = 0;
   //判断该昵称是否成功生成：如果该昵称在map里已存在那么会insert失败。
    recv(ClientScoket, RecvBuffer, sizeof(RecvBuffer), 0);
    Insert_Pair = list_socket.insert(map<string, SOCKET>::value_type(RecvBuffer, ClientScoket));
    if (Insert_Pair.second == true)
        {
        for (iter = list_socket.begin(); iter != list_socket.end(); iter++)
        {
            strcpy(SendBuffer, "欢迎");
            strcat(SendBuffer, RecvBuffer);
            strcat(SendBuffer, "加入服务器");
            send(iter->second, SendBuffer, sizeof(SendBuffer), 0);
        }
            memset(SendBuffer, 0x00, MAX_PATH);
        }
    else
    {
        send(ClientScoket, "你的用户名已被使用请重新链接,谢谢", 34, 0);
        return 0;
    }


    strcpy(SendBuffer, "现在服务器在线人员为：[");
    for (iter = list_socket.begin(); iter != list_socket.end(); iter++)
    {
        strcat(SendBuffer, iter->first.data());
        strcat(SendBuffer, "; ");
    }
    strcat(SendBuffer, "]");
    send(ClientScoket, SendBuffer, sizeof(SendBuffer), 0);
    memset(SendBuffer, 0x00, sizeof(SendBuffer));



    while (true) {
        memset(RecvBuffer, 0x00, sizeof(RecvBuffer));
        RET = recv(ClientScoket, RecvBuffer, MAX_PATH, 0);
        vector<string> result = find_SendId(RecvBuffer);
        if (RET == 0 || RET == SOCKET_ERROR)
        {
            for (iter = list_socket.begin(); iter != list_socket.end(); iter++)
            {
                while (iter->second == ClientScoket) {
                    strcpy(SendBuffer, find_key(ClientScoket).data());
                    strcat(SendBuffer, "用户掉线");
                    send(iter->second, SendBuffer, sizeof(SendBuffer), 0);

                }
                
            }
            list_socket.erase(find_key(ClientScoket));
            memset(SendBuffer, 0x00, MAX_PATH);
        }else if (strcmp(RecvBuffer, "quit")==0)
        {
            strcpy(SendBuffer, find_key(ClientScoket).data());
            strcat(SendBuffer, "用户已经成功退出");
            for (iter = list_socket.begin(); iter != list_socket.end(); iter++)
            {
                send(iter->second, SendBuffer, sizeof(SendBuffer), 0);
            }
            closesocket(ClientScoket);
            list_socket.erase(find_key(ClientScoket));
            memset(SendBuffer, 0x00, MAX_PATH);
        }else if (strcmp("all", result[0].data()) == 0)
        {
            strcpy(SendBuffer, "[");
            strcat(SendBuffer, find_key(ClientScoket).data());
            strcat(SendBuffer, "]:  ");
            strcat(SendBuffer, cut_sendbuff(RecvBuffer).data());
            for (iter = list_socket.begin(); iter != list_socket.end(); iter++)
            {
                send(iter->second, SendBuffer, sizeof(SendBuffer), 0);
            }
            memset(SendBuffer, 0x00, MAX_PATH);
        }else {
            for (int i = 0; i < result.size(); i++)
            {
                memset(SendBuffer, sizeof(SendBuffer), 0);
                strcpy(SendBuffer, "[");
                strcat(SendBuffer, find_key(ClientScoket).data());
                strcat(SendBuffer, "]:  ");
                strcat(SendBuffer, cut_sendbuff(RecvBuffer).data());
                cout << result[i] << endl;
                iter = list_socket.find(result[i]);
                if (iter != list_socket.end())
                {
                    send(list_socket[result[i]], SendBuffer, sizeof(SendBuffer), 0);
                }
                else
                {
                    memset(SendBuffer, sizeof(SendBuffer), 0);
                    strcpy(SendBuffer, result[i].data());
                    strcat(SendBuffer, " 该用户不存在");
                    send(ClientScoket, SendBuffer, sizeof(SendBuffer), 0);
                }
            }
            memset(SendBuffer, 0x00, MAX_PATH);
            
        }
    }

    return 0;
}
//主函数
int main(void)
{
    WSADATA     WSA;
    SOCKET      severSocket, clientScoket;
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
    severSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (severSocket == INVALID_SOCKET)
    {
        cout << "creat failed" << GetLastError() << endl;
        return -1;
    }
    LocalAddr.sin_family = AF_INET;
    LocalAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    LocalAddr.sin_port = htons(4000);
    memset(LocalAddr.sin_zero, 0x00, 8);
    //bind socket
    RET = bind(severSocket, (SOCKADDR*)&LocalAddr, sizeof(LocalAddr));
    if (RET != 0)
    {
        cout << "bind failed";
        return -1;
    }
    RET = listen(severSocket, 5);
    if (RET != 0)
    {
        cout << "listen failed";
        return -1;
    }
    cout << "服务器已启动" << endl;
    while (true)
    {
        AddrLen = sizeof(clientAddr);
        clientScoket = accept(severSocket, (SOCKADDR*)&clientAddr, &AddrLen);
        if (clientScoket == INVALID_SOCKET)
        {
            cout << "accept failed";
            break;
        }
        
        hThread_1 = CreateThread(NULL, 0, ClientThread, (LPVOID)clientScoket, 0, NULL);
        if (hThread_1 == NULL)
        {
            cout << "creat thread_1 failed" << endl;
        }
    }
    closesocket(severSocket);
    WSACleanup();
    return 0;
}