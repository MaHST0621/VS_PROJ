#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <bitset>
#include <pthread.h>
#include <map>
#include <time.h>
#include "Rdt.h"
using namespace std;
int g_total_window = 10;
int g_count_id = 1;
int g_shave_id = 0;
int g_acc = 0;
int g_last_str;
int g_totalpackage;
int g_Chave_id = 0;
int g_pack_length = 0;
pthread_mutex_t mutex;


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


int Rdt::get_strlen(u_char* buf)
{
    u_short sum;
    //sum = (buf[6] << 8) | buf[7];
    sum = 256*buf[6] + buf[7];
    return sum;
}



void Rdt::output_head(char* buf)
{
    for(int i = 0; i < count_head ;i++)
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
void Rdt::set_strlen()
{
    bitset<16>  test(str_length);
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
        Send_buff[2] = 1;
    }
    else if (i == 0){
        Send_buff[2] = 0;
    }

}



void Rdt::insert_buf(char* buf)
{
    for(int i = 0,j = count_head ;i < str_length;i++,j++)
    {
        Send_buff[j] = buf[i];
    }
}
//输出输入部分
void Rdt::output_buf(char* buf,char* buff)
{
    for(int i = count_head,j = 0;i < count_head + get_strlen((u_char*)buf);i++)
    {
        buff[j] = buf[i];
    }
}
//提取消息分组编号
int Rdt::get_id(u_char*  buf)
{
    u_short sum;
    sum =buf[0]*256+buf[1];
    return sum;

}
void Rdt::make_pak(int id,char* buf)
{
    memset(Send_buff,0,sizeof(Send_buff));
    if(id == g_totalpackage)
    {   str_length = g_last_str;
        g_pack_length = g_last_str + count_head;
        set_seq(1);
    }
    else
    {
        str_length = 1024;
        g_pack_length = str_length + count_head;
        set_seq(0);
    }
    set_id(id);
    set_ack(0);
    insert_buf(buf);
    cksum((u_short*)Send_buff,strlen(buf));
    set_strlen();
   
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
    // 将校验和高8位传给a
    a = sum >> 8;
    // 将校验和高8位和低8位进行交换并赋值给b
    b = sum & 0xFF;
    Send_buff[4] = a;
    Send_buff[5] = b;
    return sum ;
}


int  Rdt::check_cksum(char* buf)
{
    u_short tmp;
    tmp = (buf[4] << 8) | buf[5];
    buf[4] = 0x0;
    buf[5] = 0x0;
    u_long sum = 0;
    int count = count_head + get_strlen((u_char*)buf);
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

void set_map(int id)
{
    g_shave_id = id;
    /* if(g_shave_id <=  id) */
    /* { */
    /*     g_shave_id = id; */
    /* } */
}
