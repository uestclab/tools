#include "cmd_flag.h"
#include "spi_cmd.h"
#include "str2digit.h"

const char short_options[] = SHORT_COMMON_OPT;

const struct option long_options[] = {
	LONG_COMMON_OPT
};


static void print_usage (FILE* stream, int exit_code){
	fprintf (stream, "Usage:  spi_tool -l json_path\n");
	fprintf (stream, "Usage:  spi_tool -d spidev -i instruction\n");
    fprintf (stream, "-l : /run/media/mmcblk1p1/etc/*.json \n");
    fprintf (stream, "-d : /dev/spidev* \n");
    fprintf (stream, "-i : instruction 0x* hex\n");
	printf("usage : spi_tool [spidev] [instuction] or spi_tool [json]\n");
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

		case 'l':
			g_args->json_path = xzalloc(strlen(optarg)+1);
			memcpy(g_args->json_path,optarg,strlen(optarg)+1);
            ret = 0;
		break;

		case 'i':
			g_args->instuction = bb_strtoull(optarg, NULL, 16);
			g_args->instuction = htonl(g_args->instuction) >> 8;
            ret = 0;
		break;

		case 'd':
			g_args->spidev = xzalloc(strlen(optarg)+1);
			memcpy(g_args->spidev,optarg,strlen(optarg)+1);
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

