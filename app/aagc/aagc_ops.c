#include "tool.h"
#include "str2digit.h"
#include "gain_table.h"
#include "aagc_ops.h"
 #include <math.h>

uint32_t read_reg(int reg_addr)
{
    FILE* fp = NULL; 
    char* value = NULL; 
    int BUFSZ = 100; 
    char buf[BUFSZ]; 
    char command[150]; 
 
    sprintf(command, "devmem 0x%x", reg_addr);
	//printf("command : %s\n",command); 

    if((fp = popen(command,"r")) == NULL) 
    { 
        return 0;
    } 
    if((fgets(buf,BUFSZ,fp))!= NULL) 
    {
		value = (char*)malloc(strlen(buf)+1);
        memcpy(value,buf,strlen(buf)+1);
		// printf("get value : %s", value); 
    }
    
    pclose(fp); 
    
    fp=NULL;

	uint32_t ret = bb_strtoull(value, NULL, 16);
	ret = ret & (0x7fff);
	return ret;

}

void run_i2cset_cmd(int addr, int reg, uint8_t value)
{
    char command[128];
    sprintf(command, "i2cset -f -y 1 0x%x 0x%x 0x%x", addr, reg, value);
	system(command);
}

void write_control(uint16_t reg_high, uint16_t reg_low){
	run_i2cset_cmd(0x48,0x17,reg_high);
	usleep(1000);
	run_i2cset_cmd(0x48,0x16,reg_low);
	usleep(1000);
	run_i2cset_cmd(0x48,0x1c,0x08); // pull up bit3 complete write control
	usleep(1000);
}

void update_gain_by_table(int table_idx){
	int t_idx = table_idx;
	uint16_t reg_high = t_reg_0x17[t_idx];
	uint16_t reg_low = t_reg_0x16[t_idx];
	write_control(reg_high, reg_low);
	usleep(100000);
}

void update_gain_fine(struct aagc_state *p_state, int one_step){
	if(one_step == 1){
		p_state->control_val = p_state->control_val + 1;
	}else{
		p_state->control_val = p_state->control_val - 1;
	}

	uint16_t reg_high = (p_state->control_val & (0xfff0)) >> 4;
	uint16_t reg_low  = (p_state->control_val & (0x000f)) << 4;
	write_control(reg_high, reg_low);
	usleep(100000);
}

float get_gain_diff(){
	uint32_t signal_amp = read_reg(REG_ADDRESS);
	float gain_diff = 20.0f * log10(EXPECT_REG_VAL / ((float) signal_amp));
	return gain_diff;
}

int get_gain_step(){
	float gain_res = get_gain_diff();
	int gain_step = floor(gain_res);
	return gain_step;
}

int update_table_idx(struct aagc_state *p_state, int gain_res){
	p_state->t_idx = p_state->t_idx + gain_res;
	if(p_state->t_idx < 0){
		p_state->t_idx = 0;
		return -1;
	}else if(p_state->t_idx > T_SIZE - 1){
		p_state->t_idx = T_SIZE - 1;
		return 1;
	}
	return 0;
}

uint16_t combine_control_val(int table_idx){
	int t_idx = table_idx;
	uint16_t reg_high = t_reg_0x17[t_idx];
	uint16_t reg_low = t_reg_0x16[t_idx];
	uint16_t val = (reg_high << 4) + (reg_low >> 4);

	// printf("reg_high = %u , reg_low = %u , val = %u \n", reg_high, reg_low, val);
	return val;
}

int find_near_table_idx(uint16_t val){
	int i=0;
	for(i=0;i<T_SIZE;i++){
		uint16_t idx_control_val = combine_control_val(i);
		if(val>idx_control_val){
			return i;
		}
	}
	return T_SIZE-1;
}

void change_step(struct aagc_state *p_state, int agc_step){
	p_state->step = agc_step;
}

void caculate_rssi(float gain_res, struct aagc_state *p_state){
	float gain_if = t_if_gain[p_state->t_idx];
	p_state->rssi = -16.5 - gain_res - gain_if;
	zlog_info(p_state->zlog_handler,"rssi = %f , gain_res = %f , gain_if = %f , coarse_cnt = %u ,fine_cnt = %u\n", 
	   p_state->rssi, gain_res, gain_if, p_state->coarse_cnt, p_state->fine_cnt);
}

/* ----  aagc operation ---- */

void start(struct aagc_state *p_state){
	assert(p_state->t_idx == 0);
	// 1. max attenuation
	update_gain_by_table(p_state->t_idx);
}

/* 
	ret : 
	-1 --- max attenuation
	1  --- min attenuation
*/
int coarse_aagc(struct aagc_state *p_state){
	int gain_step = get_gain_step();
	int cnt_positive = 0;
	int cnt_negtive  = 0;
	int ret = 0;
	while(abs(gain_step) > HIGH_THRESHOLD){
		ret = update_table_idx(p_state, gain_step);
		update_gain_by_table(p_state->t_idx);
		p_state->control_val = combine_control_val(p_state->t_idx);
		gain_step = get_gain_step();
		float tmp = gain_step * 1.0;
		p_state->coarse_cnt ++;
		caculate_rssi(tmp, p_state);
		if(ret == 1){
			cnt_positive ++;
			if(cnt_positive == 10){
				return 1;
			}
		}else if(ret == -1){
			cnt_negtive ++;
			if(cnt_negtive == 10){
				return -1;
			}
		}else if(ret == 0){
			cnt_positive = 0;
			cnt_negtive  = 0;
		}
	}
	return 0;
}

/*
  ret : -1 --- back to coarse_aagc
*/
int fine_aagc(struct aagc_state *p_state){
	float gain_res = get_gain_diff();
	if(fabs(gain_res) > HIGH_THRESHOLD){
		return -1;
	}
	
	int ret = 0;
	while(fabs(gain_res) > LOW_THRESHOLD){
		int diff_one = (gain_res > 0.0f) ? -1 : 1 ; 
		update_gain_fine(p_state, diff_one);
		p_state->t_idx = find_near_table_idx(p_state->control_val);
		gain_res = get_gain_diff();
		p_state->fine_cnt++;
		caculate_rssi(gain_res, p_state);
		if(fabs(gain_res) > HIGH_THRESHOLD){
			return -1;
		}
	}
	caculate_rssi(gain_res, p_state);
	return 0;
}

// agc state machine
int agc_process_loop(struct aagc_state *p_state){
	while(1){
		switch (p_state->step){
			case START:
			{	
				zlog_info(p_state->zlog_handler,"State : START \n");
				start(p_state);
				change_step(p_state, COARSE);
				break;
			}
			case COARSE:
			{
				zlog_info(p_state->zlog_handler,"State : COARSE \n");
				int ret = coarse_aagc(p_state);
				if(ret == 0){
					change_step(p_state, FINE);
				}else if(ret == 1){
					zlog_info(p_state->zlog_handler,"cnt_positive ....... \n");
				}else if(ret == -1){
					zlog_info(p_state->zlog_handler,"cnt_negtive  ....... \n");
				}
				break;
			}
			case FINE:
			{	
				zlog_info(p_state->zlog_handler,"State : FINE \n");
				int ret = fine_aagc(p_state);
				if(ret == -1){
					change_step(p_state, COARSE);
				}else if(ret == 0){
					change_step(p_state, FINE);
					usleep(100000);
				}
				break;
			}	
			default:
				break;
		}
	}
}

void test(struct aagc_state *p_state){
	uint32_t val = read_reg(REG_ADDRESS);
	printf("read_reg = 0x%x \n\n", val);

	float gain_diff = get_gain_diff();
	printf("gain_diff = %f \n\n", gain_diff);
	int gain_step   = get_gain_step();
	printf("gain_step = %d \n\n", gain_step);

	/* pass */
	// int i;
	// for(i=0;i<T_SIZE;i++){
	// 	printf("i = %d \n", i);
	// 	uint16_t control_val = combine_control_val(i);
	// 	printf("control_val = 0x%x \n", control_val);
	// 	uint16_t high = control_val + 1;
	// 	uint16_t low  = control_val - 1;
	// 	int high_near_idx = find_near_table_idx(high);
	// 	printf("high_near_idx = %d \n", high_near_idx);
	// 	int low_near_idx = find_near_table_idx(low);
	// 	printf("low_near_idx = %d \n", low_near_idx);
	// 	printf("---------\n");
	// }

}