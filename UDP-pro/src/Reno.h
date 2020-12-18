#include "pch.h"
#ifndef Cwm_H
#define Cwm_H


//拥塞窗口数量
extern int g_cwnd;
//慢启动阈值
extern int g_ssthresh;
//在处于拥塞避免时收到的ack数
extern int g_count_ack;
//已使用的窗口数量
extern int g_send_count;

extern std::map<int,int> g_ack_count;

void set_map(int i); //用于快重传的函数，在前两次作业只是用于改变g_shave_id的函数
void avoidjam(); //拥塞避免函数
void slowstart(); //慢启动函数
void *recv_pthread(void *arg); //多线程接收函数

#endif
