#include "pch.h"
#include "Rdt.h"
#ifndef Cwm_H
#define Cwm_H
enum Status
{
    SlowStart,
    Avoid_Dup,
    Quick_Recover

};
extern bool g_Recover_Key;
extern Status g_Cwn_key;
extern double g_Mss;
extern double g_cwnd;
extern double g_ssthresh;
extern int g_base_window;
extern std::map<int,int> g_ack_count;

void set_map(int i); //用于快重传的函数，在前两次作业只是用于改变g_shave_id的函数
void *recv_pthread(void *arg); //多线程接收函数

void set_map_RENO(int i); //用于快重传的函数，在前两次作业只是用于改变g_shave_id的函数
void *recv_pthread_RENO(void *arg); //RENO多线程接收函数
bool Is_DupAck(int i);
void Cwn_DupAck(int i);
void Cwn_NewAck(int i);
void Cwn_TimeOut();
void SlowStart_Cwn(int i);
void SlowStart_NewAck_Cwn(int i);
void SlowStart_TimeOut_Cwn();
void Avoid_Dup_Cwn(int i);
void Avoid_Dup_NewAck_Cwn(int i);
void Avoid_Dup_TimeOut_Cwn();
void Quick_Recover_Cwn(int i);
void Quick_Recover_NewAck_Cwn(int i);
void Quick_Recover_TimeOut_Cwn();
#endif
