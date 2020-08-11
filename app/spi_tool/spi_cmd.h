#include "tool.h"

#define	LONG_COMMON_OPT\
	{ "help",	 0, NULL, 'h' },\
	{ "version", 0, NULL, 'v' },\
	{ "dev",    1, NULL, 'd' },\
	{ "json",   1, NULL, 'l' },\
	{ "spec",    0, NULL, 's' },\
	{ "instuction",   1, NULL, 'i' },\
	{ 0, 0, 0, 0 }

#define	SHORT_COMMON_OPT	"hvd:l:s::i:"

typedef struct g_args_para{
	char *json_path;
	char *spidev;
	uint32_t instuction;
}g_args_para;


int parse_cmd_line(int argc, char** argv, g_args_para *g_args);