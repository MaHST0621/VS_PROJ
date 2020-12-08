#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>

using namespace std;
#define SERV_PORT   8000   
#define SEND_BUFF   256

class Rdt
{
public:
    int ack_id;
    int ack = 1;
    char* Send_buff = new char[SEND_BUFF];
    

public:
    u_short cksum(u_short *buf,int count);
    void make_pak(int id,char* buf);
    int  getlen_buf(char* buf);
    u_short  get_id();


};

int Rdt::getlen_buf(char* buf)
{
    return strlen(buf);
}

u_short Rdt::get_id()
{
    for(int j=0; j < 4 ; j++)
    {

    
        for(int i=7; i >= 0; i--)
        {
            std::cout<<((Send_buff[j] >> i) & 1);
        }
    
    }
    cout<<endl;
}
void Rdt::make_pak(int id,char* buf)
{
    /* Send_buff[0] += 1; */
    /* cout<<"init:"; */
    /* for(int i = 7; i >= 0;i--) */
    /* { */
    /*     std::cout<<((Send_buff[0] >> i) &  1); */
    /* } */
    /* cout<<endl; */
    if(id == 1){
        memset(&Send_buff[0],0,sizeof(Send_buff[0]));
        Send_buff[0] += 1;
    }
    else if (id == 0){
        memset(&Send_buff[0],0,sizeof(Send_buff[0]));
    }
    /* cout<<"inin:"; */
    /* for(int i = 7; i >= 0;i--) */
    /* { */
    /*     std::cout<<((Send_buff[0] >> i) &  1); */
    /* } */
    /* cout<<endl; */

    cksum((u_short*)buf,getlen_buf(buf));
    
}
u_short Rdt::cksum(u_short *buf,int count)
{
    u_long sum = 0;
    u_char a;
    u_char b;
    while(count--)
    {
        sum += *buf++;
        if(sum & 0xFFFF0000)
        {
            sum &= 0xFFFF;
            sum++;
        }
    }
    sum = ~ (sum & 0xFFFF);
    /* cout<< "校验和："; */
    /* for(int i = 15;i >= 0;i--) */
    /* { */
    /*     std::cout<<((sum >> i) & 1) ; */
    /* } */
    /* cout<<endl; */
    // 将校验和高8位传给a
    a = sum >> 8;
    // 将校验和高8位和低8位进行交换并赋值给b
    b = ((sum >> 8 | sum << 8) >> 8);
    /* cout<<"buff:"; */
    Send_buff[1] = a;
    Send_buff[2] = b;
    /* for(int i = 7; i >= 0; i--) */
    /* { */
    /*     std::cout<< ((Send_buff[1] >> i) & 1); */
    /* } */
    /* cout<<"    "; */
    /* for(int i = 7; i >= 0; i--) */
    /* { */
    /*     std::cout<< ((Send_buff[2] >> i) & 1); */
    /* } */
    cout<<endl;
    return ~(sum & 0xFFFF);
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
  
    
    int recv_num;  
    int send_num;  
    char send_buff[SEND_BUFF];
    memset(send_buff,0,sizeof(send_buff));
    char recv_buff[20];

    struct sockaddr_in addr_client;  
    cout<<"cin:";
    cin>>send_buff;
    Rdt Rdt_udp;
    Rdt_udp.make_pak(1,send_buff);  
    while(1)  
        {  
            printf("等待链接:\n");  
                  
            recv_num = recvfrom(sock_fd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);  
                  
            if(recv_num < 0)  
                {  
                    perror("接受报错:");  
                    exit(1);  
                }  
            recv_buff[recv_num] = '\0';
            printf("server receive %d bytes: %s\n", recv_num, recv_buff);  
              
            send_num = sendto(sock_fd, Rdt_udp.Send_buff, recv_num, 0, (struct sockaddr *)&addr_client, len);  
                  
            if(send_num < 0)  
                {  
                    perror("发送报错:");  
                    exit(1);  
                }  
        }  
          
    close(sock_fd);  
            
    return 0;  
          

}
