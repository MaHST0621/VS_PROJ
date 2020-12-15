#include <pthread.h>
#ifndef __Rdt__
#define __Rdt__

extern int g_shave_id;

extern pthread_mutex_t mutex;
extern int g_total_window ;
extern int g_count_id ;
extern int g_acc;
extern int g_last_str;
extern int g_totalpackage;
extern int g_Chave_id;
extern int g_pack_length;
class Rdt
{
public:
    int count_head = 8;
    unsigned char Send_buff[1032];
    int str_length;

public:
    unsigned short cksum(unsigned short *buf,int count);
    int     check_cksum(char* buf);
    void    make_pak(int id,char* buf);
    int     get_id(unsigned char* buf);
    void    set_seq(int i);
    int     get_seq(char* buf);
    void    insert_buf(char* buf);
    void    output_buf(char* buf,char* buff);
    void    set_ack(int i);
    void    set_strlen();
    void    output_head(char* buf);
    int     get_strlen(unsigned char* buf);
    void    set_id(int i);

};

void set_map(int i);


#endif
