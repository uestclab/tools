
#include "tool.h"
#include "cJSON.h"
#include "read_file.h"
#include "str2digit.h"
#include "7417_ops.h"
#include "cmd_line.h"

void small_int2str(int val, char* str){
	char tmp[3];
	sprintf(tmp, "%d", val);
	if(strlen(tmp) == 1){
		memcpy(str + 1, tmp, strlen(str) + 1);
	}else{
		memcpy(str,tmp,strlen(str) + 1);
	}
}

void show_sensor_data(){

	// char str[128];
    // char hours[3] = {'0'};
	// char minutes[3] = {'0'};
	// char seconds[3] = {'0'};

	// small_int2str(time->hours, hours);
	// small_int2str(time->minutes, minutes);
	// small_int2str(time->seconds, seconds);

	// sprintf(str,"%s %s %d -- %s:%s:%s %d \n", weekday_str[time->weekdays], months_str[time->months-1], time->days,
	// 						 hours, minutes, seconds, time->years);
	// printf(str);

	char str[128];

	double board_temper = get_board_temper();
	double bb_voltage = get_bb_voltage();
	double bb_current = get_bb_current();
	double ADC_temper = get_ADC_temper();

	sprintf(str,"board_temperature : %f \nbb_voltage : %f \nbb_current : %f \nADC_temper : %f \n", 
			board_temper, bb_voltage, bb_current, ADC_temper);
	printf(str);
}

int main(int argc, char *argv[])
{
	if(parse_cmd_line(argc, argv)){
		return 0;
	}

	show_sensor_data();

	return 0;
}

