#include <stdio.h>   
#include <string.h>   
#include <errno.h>   
#include <stdlib.h>   
#include <unistd.h>   
#include <sys/types.h>   
#include <sys/socket.h>   
#include <netinet/in.h>   
#include <arpa/inet.h>   
#include <iostream>
#include <bitset>
#include <fstream>
#include <map>
#include "Rdt.h"
using namespace std;
#define DEST_PORT 8000   
#define DSET_IP_ADDRESS  "127.0.0.1"   



int main()  
{  
    /* socket文件描述符 */  
    int sock_fd;  
  
    /* 建立udp socket */  
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);  
    if(sock_fd < 0)  
    {  
        perror("socket");  
        exit(1);  
    }  
    
    /* 设置address */  
    struct sockaddr_in addr_serv;  
    int len;  
    memset(&addr_serv, 0, sizeof(addr_serv));  
    addr_serv.sin_family = AF_INET;  
    addr_serv.sin_addr.s_addr = inet_addr(DSET_IP_ADDRESS);  
    addr_serv.sin_port = htons(DEST_PORT);  
    len = sizeof(addr_serv);  
  
    
    int send_num;  
    int recv_num;  
    int look_num;
    int fin_id = 0;
    Rdt Client;
    u_char send_buff[15] = "i am here!!";  
    u_char send_ack[1];
    memset(send_ack,0,1);
    Client.make_pak(1,(char*)send_buff);
    u_char recv_buff[1032];  
      
    printf("client send: %s\n", send_buff);  
    std::ofstream out_result("9.jpg",std::ios::out | std::ios::binary);
    if(!out_result.is_open())
    {
        printf("文件打开失败!\n");
    }

    
    send_num = sendto(sock_fd, Client.Send_buff,sizeof(Client.Send_buff) , 0, (struct sockaddr *)&addr_serv, len);  
    if(send_num < 0)  
        {  
            perror("发送报错:");  
            exit(1);  
        }  
    else
    {
        cout<<"----------------------------------------------------已成功发送请求，准备接受文件----------------------------------------------------"<<endl;
    }
    while(1)
    {
        if(fin_id == 1)
        {
            break;
        }
        recv_num = recvfrom(sock_fd, recv_buff,1032, 0, (struct sockaddr *)&addr_serv, (socklen_t *)&len);  
        printf("收到%d号包\n",Client.get_id(recv_buff));

        printf("client receive %d bytes\n", recv_num);

        cout<<Client.get_strlen(recv_buff)<<endl;
        //Client.output_head((char*)recv_buff);
        if(recv_num < 0)  
        {  
            perror("接受报错:");  
            exit(1);  
        }  
        if(Client.get_id(recv_buff) == (g_Chave_id + 1))
        {
            if(Client.check_cksum((char*)recv_buff))
            {

                cout<<"----------------------------------------------------- 正确接受，发送ACK----------------------------------------------------"<<endl;
                if(Client.get_seq((char*)recv_buff) == 1)
                {
                    fin_id = 1;
                }
                //u_char buff[Client.get_strlen((char*)recv_buff)];
                //Client.output_buf((char*)recv_buff,(char*)buff);
                out_result.write((char*)&recv_buff[Client.count_head],Client.get_strlen(recv_buff));
                look_num = g_Chave_id + 1;
                Client.make_pak(look_num,(char*)send_ack);
                Client.set_ack(1);
                sendto(sock_fd, Client.Send_buff,8, 0, (struct sockaddr *)&addr_serv, len);  
                g_Chave_id++;
                memset(recv_buff,0,1032);
            
            }
            else
            {
                cout<<"----------------------------------------------------- 错误接受，发送ACK----------------------------------------------------"<<endl;
                look_num = g_Chave_id;
                Client.make_pak(look_num,(char*)send_ack);
                Client.set_ack(1);
                sendto(sock_fd, Client.Send_buff,8, 0, (struct sockaddr *)&addr_serv, len);  
            
                memset(recv_buff,0,1032);
            }
        
        }
        else
        {
            printf("不是期望包\n");
            continue;
        }
    }    
    close(sock_fd);  
          
    return 0;  
}

