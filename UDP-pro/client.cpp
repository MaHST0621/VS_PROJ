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
using namespace std;
   
  
#define BUFF_MX 256
#define DEST_PORT 8000   
#define DSET_IP_ADDRESS  "127.0.0.1"   
   


class Rdt
{
public:
    int ack_id;
    int count_head = 6;
    unsigned char Send_buff[BUFF_MX];
    

public:
    u_short cksum(u_short *buf,int count);
    int  check_cksum(char* buf);
    void make_pak(int id,char* buf);
    int  get_id();
    void insert_buf(char* buf);
    void output_buf(char* buf);
    void set_ack(int i);
    void set_strlen(char* buf);
    void output_head(char* buf);
    int  get_strlen(char* buf);
};

int Rdt::get_strlen(char* buf)
{
    u_short sum;
    sum = (buf[4] << 8) | buf[5];
    return (int)sum;
}



void Rdt::output_head(char* buf)
{
    for(int i = 0; i < count_head + 5;i++)
    {
        printf("head:");
        for(int j = 7;j >= 0; j--)
        {
            cout<<((buf[i] >> j) & 1);
        }
    }
    cout<<endl;
}


//将消息长度放入报文头
void Rdt::set_strlen(char* buf)
{
    int length = strlen(buf);
    bitset<16>  test(length);
    for(int i = 0 ,j = 0; i < 16;i++,j++)
    {
        if(j < 8)
        {
            if(test[i])
            {
                Send_buff[5] |= (1 << j);
            }
            else
            {
                Send_buff[5] &= ~(1 << j);
            }
        }
        if(j >= 8)
        {
            if(test[i])
            {
                Send_buff[4] |= (1 << (j-8));
            }
            else
            {
                Send_buff[4] &= ~(1 << (j-8));
            }
        }
    }
}

void Rdt::set_ack(int i)
{
    if(i == 1){
        Send_buff[1] |= (1);
    }
    else if (i == 0){
        Send_buff[1] &=  ~ (1);
    }

}



void Rdt::insert_buf(char* buf)
{
    int length = strlen(buf);
    for(int i = 0,j = count_head ;i < length;i++,j++)
    {
        Send_buff[j] = buf[i];
    }
}
//输出输入部分
void Rdt::output_buf(char* buf)
{
    for(int i = count_head;i < count_head + get_strlen(buf);i++)
    {
        cout<<buf[i];
    }
    cout<<endl;
}
//提取消息分组编号
int Rdt::get_id()
{
    if((Send_buff[0] >> 0) & 1)
        return 1;
    else
        return 0;
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
        Send_buff[0] |= 1 ;
    }
    else if (id == 0){
        Send_buff[0] &=  ~ (1);
        
    }
    /* cout<<"inin:"; */
    /* for(int i = 7; i >= 0;i--) */
    /* { */
    /*     std::cout<<((Send_buff[0] >> i) &  1); */
    /* } */
    /* cout<<endl; */
    insert_buf(buf);
    cksum((u_short*)Send_buff,strlen(buf));
    set_strlen(buf);
    set_ack(1);
    output_head((char*)Send_buff);
    cout<<endl<<check_cksum((char*)Send_buff);
    output_buf((char*)Send_buff);
   
}
u_short Rdt::cksum(u_short *buf,int count)
{
    u_long sum = 0;
    u_char a;
    u_char b;
    count = count_head + count;
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
    cout<< "校验和：";
    for(int i = 15;i >= 0;i--)
    {
        std::cout<<((sum >> i) & 1) ;
    }
    cout<<endl;
    // 将校验和高8位传给a
    a = sum >> 8;
    // 将校验和高8位和低8位进行交换并赋值给b
    b = sum & 0xFF;
    /* cout<<"buff:"; */
    Send_buff[2] = a;
    Send_buff[3] = b;
    /* for(int i = 7; i >= 0; i--) */
    /* { */
    /*     std::cout<< ((Send_buff[1] >> i) & 1); */
    /* } */
    /* cout<<"    "; */
    /* for(int i = 7; i >= 0; i--) */
    /* { */
    /*     std::cout<< ((Send_buff[2] >> i) & 1); */
    /* } */
    /* cout<<endl; */
    return sum ;
}


int  Rdt::check_cksum(char* buf)
{
    u_short tmp;
    tmp = (buf[2] << 8) | buf[3];
    buf[2] = 0x0;
    buf[3] = 0x0;
    u_long sum = 0;
    int count = 4 + get_strlen(buf);
    while(count--)
    {
        sum += *buf++;
        if(sum & 0xFFFF0000)
        {
            sum &= 0xFFFF;
            sum++;
        }
    }
    sum = ~(sum & 0xFFFF);
    if((sum) ^ (tmp))
    {
        return 1;
    }
    else
    {
        return 0;
    }

}


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
    Rdt Client;
    u_char send_buff[BUFF_MX] = "hey, who are you?";  
    Client.make_pak(1,(char*)send_buff);
    u_char recv_buff[BUFF_MX];  
      
    printf("client send: %s\n", send_buff);  
    
    send_num = sendto(sock_fd, Client.Send_buff,sizeof(Client.Send_buff) , 0, (struct sockaddr *)&addr_serv, len);  
    if(send_num < 0)  
        {  
            perror("sendto error:");  
            exit(1);  
        }  
    
    recv_num = recvfrom(sock_fd, recv_buff, sizeof(Client.Send_buff), 0, (struct sockaddr *)&addr_serv, (socklen_t *)&len);  
      
    if(recv_num < 0)  
        {  
            perror("recvfrom error:");  
            exit(1);  
        }  
    
    printf("client receive %d bytes: ", recv_num);
    Client.output_buf((char*)recv_buff);
    cout<<endl;
    Client.output_head((char*)recv_buff);
        
      close(sock_fd);  
          
        return 0;  
}

