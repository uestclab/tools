#include "cmd_flag.h"
#include "cmd_line.h"
#include "str2digit.h"

const char short_options[] = SHORT_COMMON_OPT;

const struct option long_options[] = {
	LONG_COMMON_OPT
};


static void print_usage (FILE* stream, int exit_code){
	fprintf (stream, "Usage:  rtc_tool -t 160652 -c 20200729 -w 1\n");
    fprintf (stream, "-t : hour minute second \n");
    fprintf (stream, "-c : year month day \n");
    fprintf (stream, "-w : Sunday(0) ~ Saturday(6) \n");
}

int universal_get_opt(int opt, const char *optarg, void* ptr){
	//printf("universal_get_opt : opt = %d \n",opt);
	g_args_para *g_args = (g_args_para*)ptr;

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
			g_args->time = bb_strtoull(optarg, NULL, 10);
            ret = 0;
		break;

		case 'c':
			g_args->calendar = bb_strtoull(optarg, NULL, 10);
            ret = 0;
		break;

		case 'w':
            g_args->weekday = bb_strtoull(optarg, NULL, 10);
            ret = 0;
		break;

		default: ;
	}

	return ret;
}

int parse_cmd_line(int argc, char** argv, g_args_para *g_args)
{
	g_cmd_line_para* g_cmd_line = (g_cmd_line_para*)malloc(sizeof(g_cmd_line_para));
	g_cmd_line->short_options = short_options;
	g_cmd_line->long_options = long_options;
	g_cmd_line->fun_ptr = universal_get_opt;
	g_cmd_line->ptr = g_args;

	int ret = get_cmd_line(argc, argv, g_cmd_line);

	free(g_cmd_line);
	return ret;
}

