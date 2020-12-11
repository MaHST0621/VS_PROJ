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
using namespace std;
#define SERV_PORT   8000   
#define SEND_BUFF   1038

class Rdt
{
public:
    int ack_id;
    int count_head = 8;
    unsigned char Send_buff[SEND_BUFF];
    

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
    char send_buff[1024];
    memset(send_buff,0,sizeof(send_buff));
    char recv_buff[20];
    struct sockaddr_in addr_client;  
    cout<<"cin:";
    cin>>send_buff;
    Rdt Server;
    int count_id = 0;
    ifstream file("mm.jpg",ifstream::in|ios::binary);

    if(!file.is_open())
    {
            printf("文件无法打开！\n");
            exit(1);
    }
    file.seekg(0,std::ios_base::end);
    int length = file.tellg();
    int totalpackage = length / 1024 + 1;
    printf("文件大小为%d bytes,总共有%d个数据包\n",length,totalpackage);
    file.seekg(0,std::ios_base::beg);


    while(1)  
        {  
            cout<<"----------------------------------------------------等待链接-----------------------------------------------------------------"<<endl;

            recv_num = recvfrom(sock_fd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);    
            if(recv_num < 0)  
            {  
                perror("接受报错:");  
                exit(1);  
            } 
            else
            {
                cout<<"OK!"<<endl;
            }
            
            
            while(1)
            {
                
                cout<<"test!1"<<endl;
                file.read(send_buff,1024);
                Server.make_pak(count_id,send_buff);  

                send_num = sendto(sock_fd, Server.Send_buff, sizeof(Server.Send_buff), 0, (struct sockaddr *)&addr_client, len);  
                  
                if(send_num < 0)  
                {  
                    perror("发送报错:");  
                    exit(1);  
                }  
                cout<<"----------------------------------------------------发送成功,等待反馈--------------------------------------------------------"<<endl;

                while(1)
                {                
                    recv_num = recvfrom(sock_fd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);    
                    if(recv_num < 0)  
                    {  
                        perror("接受报错:");  
                        exit(1);  
                    }  
                    if((recv_buff[1] >> 0) & 1)
                    {
                        cout<<"---------------------------------------------------------成功接受ACK---------------------------------------------------------"<<endl;
                        if(Server.get_seq((char*)recv_buff) != (count_id + 1))
                        {
                            cout<<"未能正确接受"<<endl;    
                            Server.make_pak(count_id,send_buff);  
                            send_num = sendto(sock_fd, Server.Send_buff, Server.get_strlen((char*)Server.Send_buff)+Server.count_head, 0, (struct sockaddr *)&addr_client, len);     
                            cout<<"--------------------------------------------------重传成功,等待反馈------------------------------------------------------------"<<endl;
                            continue;
                        }
                        else
                        {
                            cout<<"对方正确接受,准备发送下一个包"<<endl;
                            count_id = 0;
                            break;
                        }
                    }
                    else
                    {
                        continue;
                    }
                }
            }
        }  
          
    close(sock_fd);  
            
    return 0;  
          

}
