
#include "tool.h"
#include "cJSON.h"
#include "read_file.h"
#include "str2digit.h"
#include "aagc_ops.h"
#include "cmd_line.h"


zlog_category_t * initLog(const char* path, char* app_name){
	int rc;
	zlog_category_t *zlog_handler = NULL;

	rc = zlog_init(path);

	if (rc) {
		printf("init serverLog failed\n");
		return NULL;
	}

	zlog_handler = zlog_get_category(app_name);

	if (!zlog_handler) {
		printf("get cat fail\n");

		zlog_fini();
		return NULL;
	}

	return zlog_handler;
}

void closeLog(){
	zlog_fini();
}

int main(int argc, char *argv[])
{
	char*   prog_name;
	char*   log_file;
	int ret = parse_cmd_line(argc, argv, &prog_name, &log_file);
	if(-EPERM == ret){
		fprintf (stdout, "parse_cmd_line error : %s \n", prog_name);
        return 0;
	}
	fprintf (stdout, "cmd line : %s -l %s\n", prog_name, log_file);
	// struct aagc_state s_state = {
	// 	.t_idx = 0,
	// 	.control_val = 0x0,
	// 	.step = COARSE
	// };

	zlog_category_t *zlog_handler = initLog(log_file,prog_name);
	zlog_info(zlog_handler,"******************** start aagc ********************************\n");
	zlog_info(zlog_handler,"this version built time is:[%s  %s]\n",__DATE__,__TIME__);

	struct aagc_state *p_state = (struct aagc_state *)malloc(sizeof(struct aagc_state));
	p_state->step = START;
	p_state->t_idx = 0;
	p_state->control_val = 0x0;
	p_state->zlog_handler = zlog_handler;
	p_state->rssi = -99.0f;
	p_state->coarse_cnt = 0;
	p_state->fine_cnt   = 0;

	agc_process_loop(p_state);

	// test(p_state);

	assert(log_file == NULL);

	return 0;
}

