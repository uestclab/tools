#include "cmd_flag.h"

int get_cmd_line(int argc, char** argv, g_cmd_line_para *g_cmd_line)
{
	int ret = 0; //exitcode
	int opt;
	
	while ((opt = getopt_long(argc, argv, g_cmd_line->short_options, g_cmd_line->long_options, NULL)) != -1) {
		//ret |= universal_get_opt(opt, optarg, g_args);
		ret |= g_cmd_line->fun_ptr(opt, optarg, g_cmd_line->ptr);
	}	

	return ret;
}