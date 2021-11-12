#include "tool.h"
#include "str2digit.h"

uint8_t run_i2cget_cmd(int addr, int reg)
{
    FILE* fp = NULL; 
    char* value = NULL; 
    int BUFSZ = 100; 
    char buf[BUFSZ]; 
    char command[150]; 
 
    sprintf(command, "i2cget -f -y 1 0x%x 0x%x", addr, reg);

	//printf("command : %s\n",command); 

    if((fp = popen(command,"r")) == NULL) 
    { 
        return 0;
    } 
    if((fgets(buf,BUFSZ,fp))!= NULL) 
    {
		value = (char*)malloc(strlen(buf)+1);
        memcpy(value,buf,strlen(buf)+1); 
    }
    
    pclose(fp); 
    
    fp=NULL;

	uint8_t ret = bb_strtoull(value, NULL, 16); 
    
	return ret;
}

void run_i2cset_cmd(int addr, int reg, uint8_t value)
{
    char command[128];
    sprintf(command, "i2cset -f -y 1 0x%x 0x%x 0x%x", addr, reg, value);
	system(command);
}

void init_7516(){
	uint8_t control_1 = 0x29;
	run_i2cset_cmd(0x48,0x18,control_1);
	uint8_t control_2 = 0x15;
	run_i2cset_cmd(0x48,0x19,control_2);
	uint8_t control_3 = 0x0b;
	run_i2cset_cmd(0x48,0x1a,control_3);
	uint8_t control_4 = 0x30;
	run_i2cset_cmd(0x48,0x1b,control_4);
}

void tx_ATT(){
	uint8_t control_1 = 0x05;
	run_i2cset_cmd(0x48,0x11,control_1);
	uint8_t control_2 = 0x00;
	run_i2cset_cmd(0x48,0x10,control_2);
	uint8_t control_3 = 0x01;
	run_i2cset_cmd(0x48,0x1c,control_3);
}

void rx_ATT_first(){
	uint8_t control_1 = 0x05;
	run_i2cset_cmd(0x48,0x15,control_1);
	uint8_t control_2 = 0x00;
	run_i2cset_cmd(0x48,0x14,control_2);
	uint8_t control_3 = 0x04;
	run_i2cset_cmd(0x48,0x1c,control_3);
}

void rx_ATT_second(){
	uint8_t control_1 = 0xd7;
	run_i2cset_cmd(0x48,0x17,control_1);
	uint8_t control_2 = 0xc0;
	run_i2cset_cmd(0x48,0x16,control_2);
	uint8_t control_3 = 0x08;
	run_i2cset_cmd(0x48,0x1c,control_3);
}