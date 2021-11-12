#include "cmd_line.h"
#include "str2digit.h"

const char short_options[] = SHORT_COMMON_OPT;

const struct option long_options[] = {
	LONG_COMMON_OPT
};

static int universal_get_opt(int opt, const char *optarg, char **log_file){
	int ret = -EINVAL;
	
	switch (opt) {
		case 'v':
			fprintf (stdout, "Version: build-%s %s\n",\
				__DATE__,__TIME__);

			ret = 0;
		break;
		
		case 'h':
			ret = 0;
		break;

		case 'f':
			ret = 0;
		break;

		case 'l':
			*log_file = (char*)malloc(strlen(optarg)+1);
			if(*log_file){
				strcpy(*log_file, optarg);
                //fprintf (stdout, "optarg : %s ,log_file : %s \n", optarg, *log_file);
			}else{
				;
			}
			ret = 0;
		break;

		case 'r':
			break;

		default: ;
	}

	return ret;
}

char *get_prog_name(char *argv)
{
	int len = strlen(argv);
	int i;
	char *tmp = argv;
	
	for(i=len; i >=0; i--)
	{
		if(tmp[i] == '/'){
			i++;
			break;
		}
	}
	
	if(-1 == i){
		i = 0;
	}

	return argv + i;
}

int parse_cmd_line(int argc, char **argv, char **prog_name, char **log_file)
{
	int ret = 0; //exitcode
	int opt;
	
	*prog_name = get_prog_name(argv[0]);

	char *tmp_log_file;

	while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
		ret |= universal_get_opt(opt, optarg, &tmp_log_file);
	}	

	*log_file = tmp_log_file;
	// fprintf (stdout, "tmp_log_file : %s -- *log_file : %s \n", tmp_log_file, *log_file);

	return ret;
}

