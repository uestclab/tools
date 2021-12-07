#ifndef _AAGC_OPS_H
#define _AAGC_OPS_H
#include "zlog.h"
#include <pthread.h>

typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
typedef unsigned int		uint32_t;

enum agc_step{
    START  = 0,
    COARSE,
	FINE,
	MONITOR
};

typedef struct aagc_state{
    zlog_category_t *zlog_handler;
    int              step;             // coarse , fine
    int              t_idx;
    uint16_t         control_val;      // 12bit;
    pthread_mutex_t  rssi_mutex;
    float            rssi;
    //debug
    uint32_t         coarse_cnt;
    uint32_t         fine_cnt;
}s_aagc_state;

int agc_process_loop(struct aagc_state *p_state);
void test(struct aagc_state *p_state);

float get_rssi(struct aagc_state *p_state);
void set_rssi(struct aagc_state *p_state, float rssi);

void print_table(zlog_category_t *zlog_handler);

int parse_table(char* buf);


#endif