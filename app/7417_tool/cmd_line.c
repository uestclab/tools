#include "cmd_flag.h"
#include "cmd_line.h"
#include "str2digit.h"

const char short_options[] = SHORT_COMMON_OPT;

const struct option long_options[] = {
	LONG_COMMON_OPT
};


static void print_usage (FILE* stream, int exit_code){
	fprintf (stream, "Usage:  7417_tool\n");
	fprintf (stream, "show 7417 sensor data \n");
}

int universal_get_opt(int opt, const char *optarg, void* ptr){
	int ret = -EINVAL;
	
	switch (opt) {
		case 'v':
			fprintf (stdout, "Version: build-%s %s\n",\
				__DATE__,__TIME__);

			ret = -EPERM;
		break;
		
		case 'h':
			print_usage (stdout, 0);
			ret = -EPERM;
		break;

		case 't':
            ret = 0;
		break;

		case 'c':
            ret = 0;
		break;

		case 'w':
            ret = 0;
		break;

		default: ;
	}

	return ret;
}

int parse_cmd_line(int argc, char** argv)
{
	g_cmd_line_para* g_cmd_line = (g_cmd_line_para*)malloc(sizeof(g_cmd_line_para));
	g_cmd_line->short_options = short_options;
	g_cmd_line->long_options = long_options;
	g_cmd_line->fun_ptr = universal_get_opt;
	g_cmd_line->ptr = NULL;

	int ret = get_cmd_line(argc, argv, g_cmd_line);

	free(g_cmd_line);
	return ret;
}

