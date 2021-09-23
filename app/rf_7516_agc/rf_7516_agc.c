
#include "tool.h"
#include "cJSON.h"
#include "read_file.h"
#include "str2digit.h"
#include "7516_ops.h"
#include "cmd_line.h"

void init_agc_data(){

	char str[128];
	init_7516();
	tx_ATT();
	rx_ATT_first();
	rx_ATT_second();

	sprintf(str,"init agc 7516 complete\n");
	printf(str);
}

int main(int argc, char *argv[])
{
	if(parse_cmd_line(argc, argv)){
		return 0;
	}

	init_agc_data();

	return 0;
}
