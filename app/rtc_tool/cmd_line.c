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

static int universal_get_opt(int opt, const char *optarg, g_args_para *g_args){
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

int get_cmd_line(int argc, char **argv, g_args_para *g_args)
{
	int ret = 0; //exitcode
	int opt;
	
	while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
		ret |= universal_get_opt(opt, optarg, g_args);
	}	

	return ret;
}