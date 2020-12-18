#include "Rdt.h"
#include "Cwn.h"

int g_count_ack = 0;
int g_cwnd = 1;
int g_ssthresh = 18;  
int g_send_count = 0;
void set_map(int id)
{
    g_send_count--;
    if(g_shave_id <= id)
    {
        g_shave_id = id;
    }
    /* if(g_base_window == id - 1) */
    /* { */
    /*     pthread_mutex_lock(&mutex); */
    /*     g_total_window++; */
    /*     pthread_mutex_unlock(&mutex); */
    /*     g_base_window++; */
    /* } */
    //记录接收方返回的包序总数
    g_ack_count[id] = g_ack_count[id] + 1;
    if(g_ack_count[id] == 3)
    {
        printf("%d号包进行快速回复\n",id);
    }
    if(id < 18)
    {
        slowstart();
    }
    if(id >= 18)
    {
        avoidjam();
    }
}


//拥塞控制：慢开始函数
void slowstart()
{
    pthread_mutex_lock(&mutex);
    g_cwnd = g_cwnd + 1;
    g_total_window = g_cwnd - g_send_count;
    pthread_mutex_unlock(&mutex);
}
//拥塞控制：避免拥塞
void avoidjam()
{
    g_count_ack++;    
    if(g_count_ack == g_cwnd)
    {
        pthread_mutex_lock(&mutex);
        g_cwnd++;
        g_total_window = g_cwnd - g_send_count;
        pthread_mutex_unlock(&mutex);
        g_count_ack = 0;
    }
}
