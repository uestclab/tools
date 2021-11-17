
#include "tool.h"
#include "cJSON.h"
#include "read_file.h"
#include "str2digit.h"
#include "aagc_ops.h"
#include "cmd_line.h"
#include "gw_mosquitto/mosquitto_client.h"

int start_publish_rssi(struct aagc_state *p_state);
int debug_switch = 0; // 0 : no zlog file 

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

void info_zlog(zlog_category_t *zlog_handler, const char *format, ...)
{
	if(debug_switch == 0){
		return;
	}
	char log_buf[1024] = { 0 };
	va_list args;
	va_start(args, format);
	vsprintf(log_buf, format, args);
	va_end(args);
	zlog_info(zlog_handler, log_buf);
}

int sub_process_callback(char *buf, int buf_len, char *topic, void *userdata){
	printf(" sub_process_callback : sub_topic : %s ---- msg : %s \n", topic, buf);
	struct aagc_state *p_state = (struct aagc_state *)userdata;
	// printf(" rssi = %f, coarse_cnt = %u, fine_cnt = %u \n", p_state->rssi, p_state->coarse_cnt, p_state->fine_cnt);
	return 0;
}

int main(int argc, char *argv[])
{
	g_args_para g_args = {
		.prog_name = NULL,
		.conf_file = NULL,
		.log_file  = NULL,
		.debug_switch = 0
	}; 
	int ret = parse_cmd_line(argc, argv, &g_args);
	if(-EINVAL == ret){
		fprintf (stdout, "parse_cmd_line error : %s \n", g_args.prog_name);
        return 0;
	}else if(-EPERM == ret){
		return 0;
	}
	fprintf (stdout, "cmd line : %s -l %s\n", g_args.prog_name, g_args.log_file);
	
	debug_switch = g_args.debug_switch;

	zlog_category_t *zlog_handler = initLog(g_args.log_file,g_args.prog_name);
	zlog_info(zlog_handler,"******************** start aagc ********************************\n");
	zlog_info(zlog_handler,"this version built time is:[%s  %s]\n",__DATE__,__TIME__);

	struct aagc_state *p_state = (struct aagc_state *)malloc(sizeof(struct aagc_state));
	p_state->step = START;
	p_state->t_idx = 0;
	p_state->control_val = 0x0;
	p_state->zlog_handler = zlog_handler;
	pthread_mutex_init(&(p_state->rssi_mutex),NULL);
	p_state->rssi = -99.0f;
	p_state->coarse_cnt = 0;
	p_state->fine_cnt   = 0;

	int topic_count = 3;
	char **sub_topics = malloc((size_t )topic_count*sizeof(char *));
	sub_topics[0] = "gw/test_sub";
	sub_topics[1] = "gw/test_sub_1";
	sub_topics[2] = "gw/test_sub_2";

// struct mosq_user{
// 	char *prog_name;
// 	char **sub_topics;
// 	int topic_count;
// 	sub_cb sub_callback;
//     exception_cb exception_callback;
//     log_print log_print_callback;
// 	void *userdata;
// };

	struct mosq_user *user_config = (struct mosq_user *)malloc(sizeof(struct mosq_user));
	user_config->prog_name = strdup(g_args.prog_name);
	user_config->userdata  = (void*)p_state;
	user_config->topic_count = 3;
	user_config->sub_topics = malloc((size_t )user_config->topic_count*sizeof(char *));
	user_config->sub_topics[0] = "gw/test_sub";
	user_config->sub_topics[1] = "gw/test_sub_1";
	user_config->sub_topics[2] = "gw/test_sub_2";
	user_config->sub_callback = sub_process_callback;

	if(start_mosquitto_client(user_config) != 0){
		zlog_info(zlog_handler,"start_mosquitto_client failed !\n");
		return 0;
	}

	start_publish_rssi(p_state);
	agc_process_loop(p_state);

	// test(p_state);

	assert(g_args.log_file == NULL);

	return 0;
}

