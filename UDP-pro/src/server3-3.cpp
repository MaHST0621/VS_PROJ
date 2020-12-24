#include "Rdt.h"
#include "Reno.h"
using namespace std;
#define SERV_PORT   8000   
#define SEND_BUFF   1032


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
    clock_t start_t , finish_t;
    double duration;
    int send_num;
    char send_buff[1032];
    memset(send_buff,0,sizeof(send_buff));
    char recv_buff[20];
    struct sockaddr_in addr_client;  
    Rdt Server;
    ifstream file("test_file/mm.jpg",ifstream::in|ios::binary);
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
    file.seekg(0,std::ios_base::beg);
    while(1)  
        {  
            cout<<"----------------------------------------------------等待链接-----------------------------------------------------------------"<<endl;
            recvfrom(sock_fd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);    
            if(recv_buff[2] == 0)
            {
                printf("成功链接\n");
                /* 是一个非ACK标识包说明是链接请求，我们需要包acc标识改为1使主线程进入发送模式 */
                g_acc = 1; 
            }
            pthread_t tid;
            pthread_create(&tid,NULL,recv_pthread_RENO,(void *)(&sock_fd));
            pthread_t tid_time;
            pthread_create(&tid_time,NULL,timer_pthread_RENO,&TIME_LIMIT);
            if(g_acc != 1)
            {
                continue;
            }
            start_t = clock();
            while(1)
            {
                if(g_base_window - 1 == g_totalpackage)
                {
                    break;
                }
                pthread_mutex_lock(&mutex);
                if(g_Recover_Key)
                {
                    cout<<"快速回复"<<endl;
                    file.seekg(((g_base_window - 1) * 1024) ,std::ios_base::beg);
                    for(int i = g_base_window; i < g_count_id;i++)
                    {
                        file.read(send_buff,1024);
                        Server.make_pak(i,send_buff);
                        send_num = sendto(sock_fd, Server.Send_buff, g_pack_length, 0, (struct sockaddr *)&addr_client, len);  
                        memset(send_buff,0,1024);
                        if(send_num < 0)
                        {
                            printf("%d号包重发报错!\n",Server.get_id(Server.Send_buff));
                        }
                        else
                        {
                            printf("%d号包重发成功!\n",Server.get_id(Server.Send_buff));
                        }
                    }
                    g_Recover_Key = false;
                    usleep(TIME_LIMIT);
                }
                pthread_mutex_unlock(&mutex);
                usleep(TIME_LIMIT);

                pthread_mutex_lock(&mutex);
                
                printf("test2\n");
                if(g_count_id < g_base_window + g_cwnd && g_count_id <= g_totalpackage)
                {
                    file.read(send_buff,1024);
                    Server.make_pak(g_count_id,send_buff);
                    send_num = sendto(sock_fd, Server.Send_buff, g_pack_length, 0, (struct sockaddr *)&addr_client, len);  
                    if(send_num < 0)
                    {
                        printf("%d号包发送报错!\n",Server.get_id(Server.Send_buff));
                    }
                    else
                    {
                        printf("%d号包发送成功!\n",Server.get_id(Server.Send_buff));
                    }
                    memset(send_buff,0,1024);
                    if(g_base_window == g_count_id)
                    {
                        printf("test110\n");
                        ReSet_Timer();
                        printf("test110\n");
                    }
                    g_count_id++;
                    printf("test3\n");
                    pthread_mutex_unlock(&mutex);
                }
                else
                {
                    printf("%d,%f,%d,%f\n" ,g_count_id,g_cwnd,g_base_window,g_ssthresh);
                    printf("没有窗口可用了\n");
                    pthread_mutex_unlock(&mutex);
                }
                printf("test1\n");
                if(g_time_key)
                {
                    pthread_mutex_lock(&mutex);
                    ReSet_Timer();
                    pthread_mutex_unlock(&mutex);

                    pthread_mutex_lock(&mutex);
                    Cwn_TimeOut();
                    file.seekg(((g_base_window - 1) * 1024) ,std::ios_base::beg);
                    for(int i = g_base_window; i < g_count_id;i++)
                    {
                        file.read(send_buff,1024);
                        Server.make_pak(i,send_buff);
                        send_num = sendto(sock_fd, Server.Send_buff, g_pack_length, 0, (struct sockaddr *)&addr_client, len);  
                        memset(send_buff,0,1024);
                        if(send_num < 0)
                        {
                            printf("%d号包超时重发报错!\n",Server.get_id(Server.Send_buff));
                        }
                        else
                        {
                            printf("%d号包超时重发成功!\n",Server.get_id(Server.Send_buff));
                        }        
                    }
                    pthread_mutex_unlock(&mutex);
                }
                pthread_mutex_lock(&mutex);
                printf("base:%d, next:%d\n",g_base_window,g_count_id);
                printf("sst:%f, cwnd:%f\n",g_ssthresh,g_cwnd);
                pthread_mutex_unlock(&mutex);
            }
            break; 
        }  
    finish_t = clock();
    duration = (double)(finish_t - start_t) / 1000;  
    cout<<"吞吐率为："<<(double)(duration / (length*8));
    exit(1);
    close(sock_fd);  
            
    return 0;  
          

}
