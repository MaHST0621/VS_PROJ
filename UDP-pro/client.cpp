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
using namespace std;
map<int , int> recv_map;
int have_id = 0;
#define BUFF_MX 1300
#define DEST_PORT 8000   
#define DSET_IP_ADDRESS  "127.0.0.1"   
   

class Rdt
{
public:
    int ack_id;
    int count_head = 8;
    unsigned char Send_buff[BUFF_MX];
    

public:
    u_short cksum(u_short *buf,int count);
    int  check_cksum(char* buf);
    void make_pak(int id,char* buf);
    int  get_id(char* buf);
    void set_seq(int i);
    int  get_seq(char* buf);
    void insert_buf(char* buf);
    void output_buf(char* buf,char* buff);
    void set_ack(int i);
    void set_strlen(char* buf);
    void output_head(char* buf);
    int  get_strlen(char* buf);
    void set_id(int i);
};

void Rdt::set_id(int i)
{
    bitset<16>  test(i);
    for(int i = 0 ,j = 0; i < 16;i++,j++)
    {
        if(j < 8)
        {
            if(test[i])
            {
                Send_buff[1] |= (1 << j);
            }
            else
            {
                Send_buff[1] &= ~(1 << j);
            }
        }
        if(j >= 8)
        {
            if(test[i])
            {
                Send_buff[0] |= (1 << (j-8));
            }
            else
            {
                Send_buff[0] &= ~(1 << (j-8));
            }
        }
    }

}


void Rdt::set_seq(int i)
{
    Send_buff[3] = i;
}

int Rdt::get_seq(char* buf)
{
    return (int)buf[3];
}


int Rdt::get_strlen(char* buf)
{
    u_short sum;
    sum = (buf[6] << 8) | buf[7];
    return (int)sum;
}



void Rdt::output_head(char* buf)
{
    for(int i = 0; i < count_head;i++)
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
                Send_buff[7] |= (1 << j);
            }
            else
            {
                Send_buff[7] &= ~(1 << j);
            }
        }
        if(j >= 8)
        {
            if(test[i])
            {
                Send_buff[6] |= (1 << (j-8));
            }
            else
            {
                Send_buff[6] &= ~(1 << (j-8));
            }
        }
    }
}

void Rdt::set_ack(int i)
{
    if(i == 1){
        Send_buff[2] |= (1);
    }
    else if (i == 0){
        Send_buff[2] &=  ~ (1);
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
void Rdt::output_buf(char* buf,char* buff)
{
    for(int i = count_head,j = 0;i < count_head + get_strlen(buf);i++)
    {
        buff[j] = buf[i];
    }
}
//提取消息分组编号
int Rdt::get_id(char* buf)
{
    u_short sum;
    sum = (buf[0] << 8) | buf[1];
    return (int)sum;

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
    /* cout<<"inin:"; */
    /* for(int i = 7; i >= 0;i--) */
    /* { */
    /*     std::cout<<((Send_buff[0] >> i) &  1); */
    /* } */
    /* cout<<endl; */
    set_id(id);
    insert_buf(buf);
    cksum((u_short*)Send_buff,strlen(buf));
    set_strlen(buf);
   
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
    /* cout<< "校验和："; */
    /* for(int i = 15;i >= 0;i--) */
    /* { */
    /*     std::cout<<((sum >> i) & 1) ; */
    /* } */
    /* cout<<endl; */
    // 将校验和高8位传给a
    a = sum >> 8;
    // 将校验和高8位和低8位进行交换并赋值给b
    b = sum & 0xFF;
    /* cout<<"buff:"; */
    Send_buff[4] = a;
    Send_buff[5] = b;
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
    tmp = (buf[4] << 8) | buf[5];
    buf[4] = 0x0;
    buf[5] = 0x0;
    u_long sum = 0;
    int count = count_head + get_strlen(buf);
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
    int look_num;
    Rdt Client;
    u_char send_buff[BUFF_MX] = "i am here!!";  
    u_char send_ack[1];
    memset(send_ack,0,1);
    Client.make_pak(1,(char*)send_buff);
    u_char recv_buff[BUFF_MX];  
      
    printf("client send: %s\n", send_buff);  
    std::ofstream out_result("test2.txt",std::ios::out | std::ios::binary);
    if(!out_result.is_open())
    {
        printf("文件打开失败!\n");
    }

    
    send_num = sendto(sock_fd, Client.Send_buff,sizeof(Client.Send_buff) , 0, (struct sockaddr *)&addr_serv, len);  
    printf("test:\n");
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
        recv_num = recvfrom(sock_fd, recv_buff, Client.get_strlen((char*)recv_buff), 0, (struct sockaddr *)&addr_serv, (socklen_t *)&len);  
        printf("test:%d\n",Client.get_id((char*)recv_buff));
        printf("test1\n");
        if(recv_num < 0)  
        {  
            perror("接受报错:");  
            exit(1);  
        }  
        printf("test%d\n",have_id);
        if(Client.get_id((char*)recv_buff) == (have_id + 1))
        {
            if(Client.check_cksum((char*)recv_buff))
            {

                cout<<"----------------------------------------------------- 正确接受，发送ACK----------------------------------------------------"<<endl;
                char buff[Client.get_strlen((char*)recv_buff)];
                Client.output_buf((char*)recv_buff,buff);
                out_result.write(buff,Client.get_strlen((char*)recv_buff));
                look_num = have_id + 1;
                Client.make_pak(look_num,(char*)send_ack);
                Client.set_ack(1);
                sendto(sock_fd, Client.Send_buff,8, 0, (struct sockaddr *)&addr_serv, len);  
                have_id++;
                memset(recv_buff,0,BUFF_MX);
            
            }
            else
            {
                cout<<"----------------------------------------------------- 错误接受，发送ACK----------------------------------------------------"<<endl;
                look_num = have_id;
                Client.make_pak(look_num,(char*)send_ack);
                Client.set_ack(1);
                sendto(sock_fd, Client.Send_buff,8, 0, (struct sockaddr *)&addr_serv, len);  
            
                memset(recv_buff,0,BUFF_MX);
            }
        
        }
        else
        {
            continue;
        }
        printf("client receive %d bytes: ", recv_num);
    }    
    close(sock_fd);  
          
    return 0;  
}

