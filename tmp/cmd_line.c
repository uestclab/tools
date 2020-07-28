#include "cmd_line.h"

const char short_options[] = SHORT_COMMON_OPT;

const struct option long_options[] = {
	LONG_COMMON_OPT
};

static void print_usage (FILE* stream, int exit_code, g_args_para *g_args){
	fprintf (stream, "Usage: %s options ,eg:%s -v\n", g_args->prog_name,g_args->prog_name);
}

static int universal_get_opt(int opt, const char *optarg, g_args_para *g_args){
	int ret = -EINVAL;
	
	switch (opt) {
		case 'v':
			fprintf (stdout, "Version: build-%s %s\n",\
				__DATE__,__TIME__);

			ret = 0;
		break;
		
		case 'h':
			print_usage (stdout, 0, g_args);
			ret = -EPERM;
		break;

		case 'f':
			g_args->conf_file = (char*)malloc(strlen(optarg)+1);
			if(g_args->conf_file){
				strcpy(g_args->conf_file, optarg);
                //fprintf (stdout, "optarg : %s ,conf_file : %s \n", optarg,g_args->conf_file);
			}else{
				//
			}
			ret = 0;
		break;

		case 'l':
			g_args->log_file = (char*)malloc(strlen(optarg)+1);
			if(g_args->log_file){
				strcpy(g_args->log_file, optarg);
                //fprintf (stdout, "optarg : %s ,log_file : %s \n", optarg,g_args->log_file);
			}else{
				;
			}
			ret = 0;
		break;

		case 'r':
			// g_args->retry = bb_strtoull(optarg, NULL, 0);
			// ret = 0;
			break;

		default: ;
	}

	return ret;
}

int get_cmd_line(int argc, char **argv, g_args_para *g_args)
{
	int ret = 0; //exitcode
	int opt;
	
	g_args->prog_name = get_prog_name(argv[0]);

	while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
		ret |= universal_get_opt(opt, optarg, g_args);
	}	

	return ret;
}