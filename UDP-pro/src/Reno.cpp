#include "Rdt.h"
#include "Reno.h"

int g_count_ack = 0;
int g_cwnd = 1;
int g_ssthresh = 18;  
int g_send_count = 0;

std::map<int,int> g_ack_count;
Rdt Tread_rdt;




//多线程接受函数
void *recv_pthread(void *arg)
{
    //printf("thread working!\n");
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

                /* printf("对方已成功接受%d号包序!\n",Tread_rdt.get_id(recv_buff)); */
                int sum = Tread_rdt.get_id(recv_buff);
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


void set_map(int id)
{
    g_send_count--;
    if(g_shave_id <= id)
    {
        g_shave_id = id;
    }
    printf("滑动窗口\n");
    if(g_base_window == id - 1)
    {
    pthread_mutex_lock(&mutex);
    g_total_window++;
    pthread_mutex_unlock(&mutex);
    g_base_window++;             
    }
}


