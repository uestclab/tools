#include "tool.h"
#include "str2digit.h"

uint16_t run_i2cget_cmd(int addr, int reg, int w_flag)
{
    FILE* fp = NULL; 
    char* value = NULL; 
    int BUFSZ = 100; 
    char buf[BUFSZ]; 
    char command[150]; 
 
	if(w_flag){
		sprintf(command, "i2cget -f -y 0 0x%x 0x%x w", addr, reg);
	}else{
		sprintf(command, "i2cget -f -y 0 0x%x 0x%x", addr, reg);
	}
	//printf("command : %s\n",command); 

    if((fp = popen(command,"r")) == NULL) 
    { 
        return 0;
    } 
    if((fgets(buf,BUFSZ,fp))!= NULL) 
    {
		value = (char*)malloc(strlen(buf)+1);
        memcpy(value,buf,strlen(buf)+1);
		printf("get value : %s", value); 
    }
    
    pclose(fp); 
    
    fp=NULL;

	uint16_t ret = bb_strtoull(value, NULL, 16);
	return ret;

}

void run_i2cset_cmd(int addr, int reg, uint8_t value)
{
    char command[128];
    sprintf(command, "i2cset -f -y 0 0x%x 0x%x 0x%x", addr, reg, value);
	system(command);
}

// ad7417 地址选择
void run_i2cset_swiich_cmd(int addr, int switch_value)
{
    char command[128];
    sprintf(command, "i2cset -f -y 0 0x%x 0x%x", addr, switch_value);
	system(command);
}


/* ----  get operation ---- */

// 整板温度
double get_board_temper(){

// i2cset -y -f 0 0x28 0x00 (地址选择)
// i2cset -y -f 0 0x28 0x01 0x00
//    #等待1ms
// i2cget -y -f 0 0x28 0x00  w   #连续输出两个字节
	double temper = 0;
	run_i2cset_swiich_cmd(0x28,0x00);
	run_i2cset_cmd(0x28,0x01,0x00);
	usleep(1000);

	uint8_t reg = 0x00;
	uint16_t val = run_i2cget_cmd(0x28,reg,1);

	printf("temper : 0x%x\n", val);

	uint16_t place0 = val & (0xff);
	uint16_t place1 = (val & (0xff00)) >> 8;

	uint16_t tmp = (place0 << 2) | (place1 >> 6);
	tmp = tmp & 0x3ff;
	int msb = tmp & 0x2ff;
	if(msb){ // Negative
		temper = (tmp & 0x1ff - 512) * 1.0 / 4;
	}else{
		temper = (tmp) * 1.0 / 4;
	}

    return temper;
}

// BB电压
double get_bb_voltage(){

// i2cset -y -f 0 0x28 0x04
// i2cset -y -f 0 0x28 0x01 0x60
//    #等待1ms
// i2cget -y -f 0 0x28 0x04  w   #连续输出两个字节

	double bb_voltage = 0;

	run_i2cset_swiich_cmd(0x28,0x04);
	run_i2cset_cmd(0x28,0x01,0x60);
	usleep(1000);

	uint8_t reg = 0x04;
	uint16_t val = run_i2cget_cmd(0x28,reg,1);

	printf("bb_voltage : 0x%x\n", val);

	uint16_t place0 = val & (0xff);
	uint16_t place1 = (val & (0xff00)) >> 8;

	uint16_t tmp = (place0 << 2) | (place1 >> 6);
	tmp = tmp & 0x3ff;
	bb_voltage = tmp * 1.0 / 1024 *2.5/0.091;

    return bb_voltage;
}

// BB电流
double get_bb_current(){

// i2cset -y -f 0 0x28 0x04
// i2cset -y -f 0 0x28 0x01 0x20
//    #等待1ms
// i2cget -y -f 0 0x28 0x04  w   #连续输出两个字节
	double bb_current = 0;
	run_i2cset_swiich_cmd(0x28,0x04);
	run_i2cset_cmd(0x28,0x01,0x20);
	usleep(1000);

	uint8_t reg = 0x04;
	uint16_t val = run_i2cget_cmd(0x28,reg,1);

	printf("bb_current : 0x%x\n", val);

	uint16_t place0 = val & (0xff);
	uint16_t place1 = (val & (0xff00)) >> 8;

	uint16_t tmp = (place0 << 2) | (place1 >> 6);
	tmp = tmp & 0x3ff;

	bb_current = tmp * 1.0 / 1024 *2.5/0.2;
    
    return bb_current;

}

// ADC温度
double get_ADC_temper(){

// i2cset -y -f 0 0x28 0x04
// i2cset -y -f 0 0x28 0x01 0x80
//    #等待1ms
// i2cget -y -f 0 0x28 0x04  w   #连续输出两个字节
	double ADC_temper = 0;
	run_i2cset_swiich_cmd(0x28,0x04);
	run_i2cset_cmd(0x28,0x01,0x80);
	usleep(1000);

	uint8_t reg = 0x04;
	uint16_t val = run_i2cget_cmd(0x28,reg,1);

	printf("ADC_temper : 0x%x\n", val);

	uint16_t place0 = val & (0xff);
	uint16_t place1 = (val & (0xff00)) >> 8;

	uint16_t tmp = (place0 << 2) | (place1 >> 6);
	tmp = tmp & 0x3ff;

    ADC_temper = (0.689 - tmp * 1.0 / 1024 * 2.5) /0.0019;
    
	return ADC_temper;

}