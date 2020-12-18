#include "pch.h"
#ifndef __Rdt__
#define __Rdt__
//发送端收到的最大ACK
extern int g_shave_id;
//线程锁，避免调用全局变量是与主线程发生冲突
extern pthread_mutex_t mutex;
//可用的滑动窗口数量
extern int g_total_window ;
//发送的包序
extern int g_count_id ;
//面向链接成功后会会触发发送状态的变量
extern int g_acc;
//最后一个包的字节数
extern int g_last_str;
//总共要发送的包数量
extern int g_totalpackage;
//客户端正确收到并且有序的包数量
extern int g_Chave_id;
//发送端发送的包长度
extern int g_pack_length;
extern int g_base_window;
extern char g_window_key;

class Rdt
{
public:
    int count_head = 8; //报文头长度
    unsigned char Send_buff[1032]; //类完成打包的报文的变量
    int str_length; //输入报文的长度（不包含报文头）

public:
    unsigned short cksum(unsigned short *buf,int count); //校验和的运算函数
    int     check_cksum(char* buf); //校验和的验证函数
    void    make_pak(int id,char* buf); //打包报文的函数
    int     get_id(unsigned char* buf); //获取包序的函数
    void    set_seq(int i); //设置包类型的标识（在这里只是用于标记最后一个包）
    int     get_seq(char* buf); //获取包类型的函数
    void    insert_buf(char* buf); //将报文头和报文的链接函数
    void    output_buf(char* buf,char* buff); //提取除了报文头以外的报文函数
    void    set_ack(int i); //设置包是否包含ACK信息
    void    set_strlen(); //将除报文长度写入报文头的函数
    void    output_head(char* buf); //打印包报文头的函数（只是用于测试）
    int     get_strlen(unsigned char* buf); //从报文头获取需要写入的报文长度
    void    set_id(int i); //设置包序的函数

};



#endif
