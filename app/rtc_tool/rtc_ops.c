#include "tool.h"
#include "str2digit.h"

uint8_t run_i2cget_cmd(int addr, int reg)
{
    FILE* fp = NULL; 
    char* value = NULL; 
    int BUFSZ = 100; 
    char buf[BUFSZ]; 
    char command[150]; 
 
    sprintf(command, "i2cget -f -y 0 0x%x 0x%x", addr, reg);
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
    sprintf(command, "i2cset -f -y 0 0x%x 0x%x 0x%x", addr, reg, value);
	system(command);
}

void stop_clock(){
	uint8_t control_1 = 0x20;
	run_i2cset_cmd(0x51,0x00,control_1); // stop bit set 1
	//uint8_t control_2 = 0x00;
}

void start_clock(){
	uint8_t control_1 = 0x00;
	run_i2cset_cmd(0x51,0x00,control_1); // stop bit set 1
}

// 00 ~ 99
void set_years(int years){ //2020 set 2020
	uint8_t reg = 0x08;
	uint8_t val = run_i2cget_cmd(0x51,reg);

    int tmp  = years - (years / 100) * 100;

	uint8_t place0 = tmp % 10;
	uint8_t place1 = tmp / 10;

	val &= (0x00);
	val |= (place0);
	val |= (place1<<4);

	run_i2cset_cmd(0x51,reg,val);
}

// 01 ~ 12
void set_months(int months){
	uint8_t reg = 0x07;
	uint8_t val = run_i2cget_cmd(0x51,reg);

    int tmp = months;
	uint8_t place0 = tmp - (tmp / 10) * 10;
    tmp = tmp / 10;
	uint8_t place1 = tmp;

	val &= (~(0x1f));
	val |= (place0);
	val |= (place1<<4);

	run_i2cset_cmd(0x51,reg,val);
}

// 0 SunDay ~ 6 Saturday
void set_weekdays(int weekdays){
	uint8_t reg = 0x06;
	uint8_t val = run_i2cget_cmd(0x51,reg);

	uint8_t place0 = weekdays % 10;

	val &= (~(0x7));
	val |= (place0);

	run_i2cset_cmd(0x51,reg,val);
}

// 01 ~ 31 or 28 or 29 or 30 leap year
void set_days(int days){
	uint8_t reg = 0x05;
	uint8_t val = run_i2cget_cmd(0x51,reg);

	uint8_t place0 = days % 10;
	uint8_t place1 = days / 10;

	val &= (~(0x3f));
	val |= (place0);
	val |= (place1<<4);

	run_i2cset_cmd(0x51,reg,val);
}

// 00 ~ 23
void set_hours(int hours){
	uint8_t reg = 0x04;
	uint8_t val = run_i2cget_cmd(0x51,reg);

	uint8_t place0 = hours % 10;
	uint8_t place1 = hours / 10;

	val &= (~(0x3f));
	val |= (place0);
	val |= (place1<<4);

	run_i2cset_cmd(0x51,reg,val);
}

// 0-59
void set_minutes(int minutes){
	uint8_t reg = 0x03;
	uint8_t val = run_i2cget_cmd(0x51,reg);

	uint8_t place0 = minutes % 10;
	uint8_t place1 = minutes / 10;

	val &= (~(0x7f));
	val |= (place0);
	val |= (place1<<4);

	run_i2cset_cmd(0x51,reg,val);
}

// 0-59
void set_seconds(int seconds){
	uint8_t reg = 0x02;
	uint8_t val = run_i2cget_cmd(0x51,reg);

	uint8_t place0 = seconds % 10;
	uint8_t place1 = seconds / 10;

	val &= (~(0x7f));
	val |= (place0);
	val |= (place1<<4);

	run_i2cset_cmd(0x51,reg,val);
}

/* ----  get operation ---- */

// 00 ~ 99
int get_years(){
	uint8_t reg = 0x08;
	uint8_t val = run_i2cget_cmd(0x51,reg);

	uint8_t place0 = val & (0x0f);
	uint8_t place1 = (val & (0xf0)) >> 4;

    int years = 2000 + place1 * 10 + place0;
    return years;
}

// 01 ~ 12
int get_months(){
	uint8_t reg = 0x07;
	uint8_t val = run_i2cget_cmd(0x51,reg);

	uint8_t place0 = val & (0x0f);
	uint8_t place1 = (val & (0x10))>>4;

    int months = place1 * 10 + place0;
    return months;
}

// 0 SunDay ~ 6 Saturday
int get_weekdays(){
	uint8_t reg = 0x06;
	uint8_t val = run_i2cget_cmd(0x51,reg);

	uint8_t place0 = val & (0x07);

    int weekdays = place0;
    return weekdays;
}

// 01 ~ 31 or 28 or 29 or 30 leap year
int get_days(){
	uint8_t reg = 0x05;
	uint8_t val = run_i2cget_cmd(0x51,reg);

	uint8_t place0 = val & (0x0f);
    uint8_t place1 = (val & (0x30))>>4;

    int days = place0 + place1 * 10;
    return days;
}

// 00 ~ 23
int get_hours(){
	uint8_t reg = 0x04;
	uint8_t val = run_i2cget_cmd(0x51,reg);

	uint8_t place0 = val & (0x0f);
    uint8_t place1 = (val & (0x30))>>4;

    int hours = place0 + place1 * 10;
    return hours;
}

// 0-59
int get_minutes(){
	uint8_t reg = 0x03;
	uint8_t val = run_i2cget_cmd(0x51,reg);

	uint8_t place0 = val & (0x0f);
    uint8_t place1 = (val & (0x70))>>4;

    int minutes = place0 + place1 * 10;
    return minutes;
}

// 0-59
int get_seconds(){
	uint8_t reg = 0x02;
	uint8_t val = run_i2cget_cmd(0x51,reg);

	uint8_t place0 = val & (0x0f);
    uint8_t place1 = (val & (0x70))>>4;

    int seconds = place0 + place1 * 10;
    return seconds;
}