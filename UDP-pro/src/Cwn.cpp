#include "Rdt.h"
void set_map(int id)
{
    if(g_shave_id <= id)
    {
        g_shave_id = id;
    }
    if(g_base_window == id - 1)
    {
        pthread_mutex_lock(&mutex);
        g_total_window++;
        pthread_mutex_unlock(&mutex);
        g_base_window++;
    }
    //记录接收方返回的包序总数
    g_ack_count[id] = g_ack_count[id] + 1;
}
