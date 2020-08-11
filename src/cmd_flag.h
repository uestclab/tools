#include "tool.h"

// cmd_flag processed by user setting
//int universal_get_opt(int opt, const char *optarg, g_args_para *g_args)

typedef int (*PROC_CALLBACK_FUN)(int opt, const char *optarg, void *g_args);

typedef struct g_cmd_line_para{
	const char *short_options;
	const struct option *long_options;
	PROC_CALLBACK_FUN fun_ptr;
	void* ptr;
}g_cmd_line_para;

int get_cmd_line(int argc, char** argv, g_cmd_line_para *g_cmd_line);