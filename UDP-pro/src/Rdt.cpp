#include "pch.h"
#include "Rdt.h"
using namespace std;
//初始化滑动窗口的大小，当为1时是停等机制
int g_total_window = 10;
int g_count_id = 1;
int g_shave_id = 0;
int g_acc = 0;
int g_last_str;
int g_totalpackage;
int g_Chave_id = 0;
int g_pack_length = 0;
int g_base_window = 0;  
pthread_mutex_t mutex;
map<int,int> g_ack_count;
//将包序号的int型用bitset转换为16位，并通过按位操作进行赋值
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


//与放包序函数一样将消息长度放入报文头
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
int Rdt::get_id(u_char*  buf)
{
    u_short sum;
    sum =buf[0]*256+buf[1];
    return sum;

}

void Rdt::make_pak(int id,char* buf)
{
    memset(Send_buff,0,sizeof(Send_buff));
    /* 判断设置的包序是否是最后一个包，如果是那么我们将写入长度改为已经算好的值，并将其标记为最后一个包 */
    if(id == g_totalpackage)
    {   str_length = g_last_str;
        g_pack_length = g_last_str + count_head;
        set_seq(1);
    }
    /* 如果不是最后一个包，那么我们将以规定好的包长度发送 */
    else
    {
        str_length = 1024;
        g_pack_length = str_length + count_head;
        set_seq(0);
    }
    set_id(id);
    set_ack(0); //这儿只用于发送端，接受到会在自己主线程设置ACK标志
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
    // 将校验和低8位传给a
    b = sum & 0xFF;
    Send_buff[4] = a;
    Send_buff[5] = b;
    return sum ;
}

//提取包中的16位校验和并清零再算一次校验和与之前提取的校验和做取反操作判断是否一样
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
/*--------------------------------------------------------------------------类函数结束------------------------------------------------------------------------------------*/

