#include "Reno.h"
int g_count_ack = 0;
double g_cwnd = 1;
double g_Mss = 1;
int TIME_LIMIT = 15000;
int g_base_window = 1;
double g_ssthresh = 18;  
Status g_Cwn_key = SlowStart;
bool g_Recover_Key = false;
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
            if(Tread_rdt.check_cksum(recv_buff))
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

/* ------------------------------------------------------------------3-3相关函数---------------------------------------------------------------------------------------- */
bool Is_DupAck(int i)
{
    std::map<int, int>::iterator it = g_ack_count.find(i);
    return it != g_ack_count.end();
}
//慢启动时Ack操作函数
void SlowStart_Cwn(int i)
{
    if(!Is_DupAck(i))
    {
        return;
    }
    g_ack_count[i]++;
    if(g_ack_count[i] >= 4)
    {
        g_ssthresh = g_cwnd / 2;
        g_cwnd = g_ssthresh + 3;
        g_Recover_Key = true;

        g_Cwn_key = Quick_Recover;
    }
}
//慢启动时收到新的Ack
void SlowStart_NewAck_Cwn(int i)
{
    if(Is_DupAck(i))
    {
        return;
    }

    g_cwnd = g_cwnd + 1;
    g_ack_count.clear();
    g_Recover_Key = false;
    g_ack_count[i] = 1;
    if(g_cwnd >= g_ssthresh)
    {
        g_Cwn_key = Avoid_Dup;
    }
}
//慢启动超时处理
void SlowStart_TimeOut_Cwn()
{
    g_ssthresh = g_cwnd / 2;
    g_cwnd = 1;
    g_Recover_Key = false;
    g_ack_count.clear();
}
//快速回复Ack处理函数
void Quick_Recover_Cwn(int i)
{
    if(!Is_DupAck(i))
    {
        return;
    }
    printf("quick_dup-1\n");
    g_cwnd = g_cwnd + 1;
}
//快速回复收到新的ACK
void Quick_Recover_NewAck_Cwn(int i)
{
    if(Is_DupAck(i))
    {
        return;
    }
    g_cwnd = g_ssthresh;
    g_ack_count.clear();
    g_Recover_Key = false;
    g_ack_count[i] = 1;
    g_Cwn_key = Avoid_Dup;
}
//快速回复包序号超时
void Quick_Recover_TimeOut_Cwn()
{
    g_ssthresh = g_cwnd / 2;
    g_cwnd = 1;
    g_ack_count.clear();
    g_Recover_Key = false;
    g_Recover_Key = SlowStart;
}
//拥塞避免在冗余Ack时的操作
void Avoid_Dup_Cwn(int i)
{
    if(!Is_DupAck(i))
    {
        return;
    }
    g_ack_count[i]++;
    if(g_ack_count[i] == 4)
    {
        g_ssthresh = g_cwnd / 2;
        g_cwnd = g_ssthresh + 3;
        g_Recover_Key = true;
        g_Cwn_key = Quick_Recover;
    }
    printf("avoid_ack_count%d\n",g_ack_count[i]);
}
//拥塞避免遇到新的Ack操作
void Avoid_Dup_NewAck_Cwn(int i)
{
    if(Is_DupAck(i))
    {
        return;
    }
    g_cwnd = g_cwnd + g_Mss * g_Mss / g_cwnd;
    g_ack_count.clear();
    g_Recover_Key = false;
    g_ack_count[i] = 1;

}
//拥塞避免包序超时
void Avoid_Dup_TimeOut_Cwn()
{
    g_ssthresh = g_cwnd / 2;
    g_cwnd = 1;
    g_ack_count.clear();
    g_Recover_Key = false;

    g_Cwn_key = SlowStart;
}
void Cwn_DupAck(int i)
{
    switch (g_Cwn_key)
    {
    case SlowStart:{
                       printf("slowstart_dup\n");
                       return SlowStart_Cwn(i);
                   }
    case Avoid_Dup:{
                       printf("Avoid_dup\n");
                       return Avoid_Dup_Cwn(i);
                   }
    case Quick_Recover:{
                            printf("Qucik_dup\n");
                            return Quick_Recover_Cwn(i);
                       }
    }
    
}
void Cwn_NewAck(int i)
{
    switch (g_Cwn_key){
    case SlowStart:{
                        printf("slowstart_new\n");
                        return SlowStart_NewAck_Cwn(i);
                   }
    case Avoid_Dup:{
                        printf("Avoidt_new\n");
                        return Avoid_Dup_NewAck_Cwn(i);
                   }
    case Quick_Recover:{
                        printf("quick_new\n");
                        return Quick_Recover_NewAck_Cwn(i);
                       }
    }
}
void Cwn_TimeOut()
{
    switch (g_Cwn_key){
    case SlowStart:{
                       printf("slowstart_timeou\n");
                       return SlowStart_TimeOut_Cwn(); 
                   }
    case Avoid_Dup:{
                       return Avoid_Dup_TimeOut_Cwn();
                   }
    case Quick_Recover:{
                           return Quick_Recover_TimeOut_Cwn();
                       }
    }
}
void set_map_RENO(int id)
{
    if(g_shave_id <= id)
    {
        g_shave_id = id;
    }
    printf("拥塞控制\n");
    g_base_window = id + 1;
    if(Is_DupAck(id))
    {
        printf("Dup_Ack\n");
        Cwn_DupAck(id); 
    }
    else
    {
        printf("New_Ack\n");
        Cwn_NewAck(id);
    }
    if(g_base_window == g_count_id)
    {
        g_time_key = false;
    }
    else
    {
        ReSet_Timer();
    }
}
//RENO多线程接受函数
void *recv_pthread_RENO(void *arg)
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
            if(Tread_rdt.check_cksum(recv_buff))
            {

                printf("对方已成功接受%d号包序!\n",Tread_rdt.get_id(recv_buff));
                int sum = Tread_rdt.get_id(recv_buff);
                pthread_mutex_lock(&mutex);
                set_map_RENO(sum);    
                pthread_mutex_unlock(&mutex);
            }
            else
                continue;
        }
        else
            printf("包信息获取错误！\n");
            continue;
    }
}

/*---------------------------------------------------------------------------Timer----------------------------------------------------------------------------*/
bool g_time_key = false;
clock_t g_begin;
clock_t g_end;
void *timer_pthread_RENO(void *arg)
{
    int Time = *(int*)arg;
    g_begin = clock();
    pthread_mutex_lock(&mutex);
    g_time_key = false;
    pthread_mutex_unlock(&mutex);

    while(1)
    {
        g_end = clock();
        if((g_end - g_begin) > Time)
        {
            printf("发生超时！\n");
            pthread_mutex_lock(&mutex);
            g_time_key = true;
            pthread_mutex_unlock(&mutex);
        }
        usleep(Time / 10);
    }
    return 0;
}


void ReSet_Timer()
{
    g_begin = clock();
    g_time_key = false;
}
