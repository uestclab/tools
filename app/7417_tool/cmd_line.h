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

int parse_cmd_line(int argc, char** argv);