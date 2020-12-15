#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <bitset>
#include <fstream>
#include <pthread.h>
#include <map>
#include <time.h>
#include "Rdt.h"
using namespace std;
#define SERV_PORT   8000   
#define SEND_BUFF   1032


Rdt Tread_rdt;

void *recv_pthread(void *arg)
{
    printf("thread working!\n");
    u_char recv_buff[20];
    int sockid = (*(int *)arg);
    
    struct sockaddr_in src;
    socklen_t len = sizeof(src);
    while(1)
    {
        int recv_num = recvfrom(sockid,recv_buff, Tread_rdt.get_strlen(recv_buff) + Tread_rdt.count_head, 0, (struct sockaddr *)&src, (socklen_t *)&len);    
        printf("收到%d号包的反馈\n",Tread_rdt.get_id(recv_buff));
        if(recv_num < 0)
        {
            printf("接收报错！\n");
        }
        if(recv_buff[2] == 1)
        {
            if(Tread_rdt.check_cksum((char*)recv_buff))
            {

     //           printf("对方已成功接受%d号包序!\n",Tread_rdt.get_id(recv_buff));
                int sum = Tread_rdt.get_id(recv_buff);
                pthread_mutex_lock(&mutex);
                g_total_window++;
                
                pthread_mutex_unlock(&mutex);
                set_map(sum);    
            }
            else
                continue;
        }
        else
            printf("包信息获取错误！\n");
            continue;
    }
}



int main()  
{  
  /* sock_fd --- socket文件描述符 创建udp套接字*/  
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_fd < 0)  
    {  
        perror("socket");  
        exit(1);  
    }  
  
    struct sockaddr_in addr_serv;  
    int len;  
    /*每个字节都用0填充*/
    memset(&addr_serv, 0, sizeof(struct sockaddr_in)); 
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_port = htons(SERV_PORT); 
    /* INADDR_ANY表示不管是哪个网卡接收到数据，只要目的端口是SERV_PORT，就会被该应用程序接收到 */  
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);  //自动获取IP地址
    len = sizeof(addr_serv);  
    
    /* 绑定socket */  
    if(bind(sock_fd, (struct sockaddr *)&addr_serv, sizeof(addr_serv)) < 0)  
    {  
        perror("bind error:");  
        exit(1);  
    }   
  
    int send_num;
    char send_buff[1032];
    memset(send_buff,0,sizeof(send_buff));
    char recv_buff[20];
    struct sockaddr_in addr_client;  
    Rdt Server;
    ifstream file("test_file/1.jpg",ifstream::in|ios::binary);
    if(!file.is_open())
    {
            printf("文件无法打开！\n");
            exit(1);
    }
    file.seekg(0,std::ios_base::end);
    int length = file.tellg();
    g_totalpackage = length / 1024 + 1;
    g_last_str =  length - ((g_totalpackage - 1)* 1024 );
    printf("文件大小为%d bytes,总共有%d个数据包,最后一个包大小为%d\n",length,g_totalpackage,g_last_str);
    if(g_total_window >= g_totalpackage)
    {
        g_total_window = g_totalpackage;
    }
    file.seekg(0,std::ios_base::beg);
       while(1)  
        {  
            cout<<"----------------------------------------------------等待链接-----------------------------------------------------------------"<<endl;
            recvfrom(sock_fd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);    
            if(recv_buff[2] == 0)
            {
                printf("成功链接\n");
                g_acc = 1;
            }
            pthread_t tid;
            pthread_create(&tid,NULL,recv_pthread,(void *)(&sock_fd));
            while(1)
            {
                if(g_acc != 1)
                {
                    continue;
                }
                if(g_shave_id == g_totalpackage)
                {
                    break;
                }
                printf("进入发送循环\n");
                while(g_total_window != 0)
                {
                    if(g_count_id > g_totalpackage)
                    {
                        break;
                    }
                    file.read(send_buff,1024);
                    Server.make_pak(g_count_id,send_buff);
                    send_num = sendto(sock_fd, Server.Send_buff, g_pack_length, 0, (struct sockaddr *)&addr_client, len);  

                    if(send_num < 0)
                    {
                        printf("%d号包发送报错！！\n",Server.get_id(Server.Send_buff));
                        cout<<strerror(errno);
                    }
                    else
                    {
                        printf("成功发送%d号包\n",Server.get_id(Server.Send_buff));
                    }
                    g_count_id++;
                    g_total_window--;
                    memset(send_buff,0,1024);
                }
                sleep(3);
                while(g_total_window == 0)
                {
                    printf("no\n");
                    if(g_shave_id == g_totalpackage)
                    {
                        break;
                    }
                    file.seekg((g_shave_id * 1024) ,std::ios_base::beg);
                    for(int i = g_shave_id ; i < g_count_id;i++)
                    {
                        if(g_shave_id == 0)
                        {
                            i = i+1;
                        }
                        file.read(send_buff,1024);
                        Server.make_pak(i,send_buff);
                        send_num = sendto(sock_fd, Server.Send_buff, Server.count_head + Server.get_strlen(Server.Send_buff), 0, (struct sockaddr *)&addr_client, len);  
                        if(send_num < 0)
                        {
                        printf("发送报错！！\n");
                        }
                        printf("重发%d号包\n",Server.get_id(Server.Send_buff));
                        memset(Server.Send_buff,0,sizeof(Server.Send_buff));
                    }
                }  

            }
            break;
        }  
    
    exit(1);
    close(sock_fd);  
            
    return 0;  
          

}
