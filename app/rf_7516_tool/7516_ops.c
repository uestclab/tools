#include "tool.h"
#include "str2digit.h"

uint8_t run_i2cget_cmd(int addr, int reg)
{
    FILE* fp = NULL; 
    char* value = NULL; 
    int BUFSZ = 100; 
    char buf[BUFSZ]; 
    char command[150]; 
 
#ifdef ZYNQ
    sprintf(command, "i2cget -f -y 1 0x%x 0x%x", addr, reg);
#endif

#ifdef ZYNQ_MP
    sprintf(command, "i2cget -f -y 0 0x%x 0x%x", addr, reg);
#endif
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
#ifdef ZYNQ
    sprintf(command, "i2cset -f -y 1 0x%x 0x%x 0x%x", addr, reg, value);
#endif

#ifdef ZYNQ_MP
	sprintf(command, "i2cset -f -y 0 0x%x 0x%x 0x%x", addr, reg, value);
#endif
	system(command);
}

void init_7516(){
	uint8_t control_1 = 0x29;
	run_i2cset_cmd(0x48,0x18,control_1);
	uint8_t control_2 = 0x14;
	run_i2cset_cmd(0x48,0x19,control_2);
	uint8_t control_3 = 0x0b;
	run_i2cset_cmd(0x48,0x1a,control_3);
}

float get_system_current(){
	uint8_t control_1 = 0x12;
	run_i2cset_cmd(0x48,0x19,control_1);

	uint8_t high8bit = run_i2cget_cmd(0x48,0x08);
	uint8_t low2bit = run_i2cget_cmd(0x48,0x04);

	uint16_t value = high8bit;
	value = value << 2;
	low2bit = low2bit & (0x3);
	value = value | low2bit;

	float ret = (value * 1.0 / 1024) * 2.28 / 60 / 0.025;
	return ret;
}

float get_PA_DET(){
	uint8_t control_1 = 0x15;
	run_i2cset_cmd(0x48,0x19,control_1);

	uint8_t high8bit = run_i2cget_cmd(0x48,0x0B);
	uint8_t low2bit = run_i2cget_cmd(0x48,0x04);

	uint16_t value = high8bit;
	value = value << 2;
	low2bit = low2bit & (0xc0);
	low2bit = low2bit >> 6;
	value = value | low2bit;

	float ret = (value * 1.0 / 1024) * 2.28;
	return ret;	
}

float get_if_temp(){
	uint8_t control_1 = 0x13;
	run_i2cset_cmd(0x48,0x19,control_1);

	uint8_t high8bit = run_i2cget_cmd(0x48,0x09);
	uint8_t low2bit = run_i2cget_cmd(0x48,0x04);

	uint16_t value = high8bit;
	value = value << 2;
	low2bit = low2bit & (0xc);
	low2bit = low2bit >> 2;
	value = value | low2bit;

	float ret = (value * 1.0 / 1024) * 2.28;
	ret = (ret - 0.5) / 0.01;
	return ret;	
}

float get_rf_temp(){
	uint8_t control_1 = 0x14;
	run_i2cset_cmd(0x48,0x19,control_1);

	uint8_t high8bit = run_i2cget_cmd(0x48,0x0A);
	uint8_t low2bit = run_i2cget_cmd(0x48,0x04);

	uint16_t value = high8bit;
	value = value << 2;
	low2bit = low2bit & (0x30);
	low2bit = low2bit >> 4;
	value = value | low2bit;

	float ret = (value * 1.0 / 1024) * 2.28;
	ret = (ret - 0.5) / 0.01;
	return ret;	
}