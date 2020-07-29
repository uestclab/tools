#include "tool.h"

#define	LONG_COMMON_OPT\
	{ "help",	 0, NULL, 'h' },\
	{ "version", 0, NULL, 'v' },\
	{ "time",    1, NULL, 't' },\
	{ "weekday",   1, NULL, 'w' },\
	{ "spec",    0, NULL, 's' },\
	{ "calendar",   1, NULL, 'c' },\
	{ 0, 0, 0, 0 }

#define	SHORT_COMMON_OPT	"hvt:w:s::c:"

typedef struct g_args_para{
	int weekday;
	uint32_t calendar; // 20200729
	uint32_t time; // 155156 //
}g_args_para;


int get_cmd_line(int argc, char** argv, g_args_para *g_args);