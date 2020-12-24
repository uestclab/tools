
#include "tool.h"
#include "cJSON.h"
#include "read_file.h"
#include "str2digit.h"
#include "7516_ops.h"
#include "cmd_line.h"

void show_rf_data(){

	char str[128];
	init_7516();
	float system_current = get_system_current();
	float PA_DET = get_PA_DET();
	float if_temp = get_if_temp();
	float rf_temp = get_rf_temp();

	sprintf(str,"system_current : %f \nPA_DET : %f \nIF_temper : %f \nRF_temper : %f \n", 
			system_current, PA_DET, if_temp, rf_temp);
	printf(str);
}

int main(int argc, char *argv[])
{
	if(parse_cmd_line(argc, argv)){
		return 0;
	}

	show_rf_data();

	return 0;
}
