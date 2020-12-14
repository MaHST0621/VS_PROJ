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


int Rdt::get_strlen(char* buf)
{
    u_short sum;
    sum = (buf[6] << 8) | buf[7];
    return (int)sum;
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
    for(int i = count_head,j = 0;i < count_head + get_strlen(buf);i++)
    {
        buff[j] = buf[i];
    }
}
//提取消息分组编号
int Rdt::get_id(u_char*  buf)
{
    u_short sum;
    //sum = (buf[0] << 8) | buf[1];
    sum =buf[0]*256+buf[1];
    return sum;

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
    memset(Send_buff,0,sizeof(Send_buff));
    if(id == g_totalpackage)
        str_length = g_last_str;
    else
    {
        str_length = 1024;
    }
    set_id(id);
    set_ack(0);
    set_seq(0);
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

void set_map(int id)
{
    g_shave_id = id;
    /* int j = id - have_id; */
    /* if(j > 0) */
    /* { */ 
    /*     for(int i = j; i > 0;i--) */
    /*     { */
    /*         maprecv[have_id] = 1; */
    /*         have_id++; */
    /*     } */
    /* } */
    /* else */ 
    /*     return; */
}
