
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <time.h>

#include "mqtt_protocol.h"
#include "mosquitto.h"
#include "client_shared.h"

#include "mosquitto_client.h"


#define MSG_MAX_SIZE  512

#define STATUS_CONNECTING 0
#define STATUS_CONNACK_RECVD 1
#define STATUS_WAITING 2
#define STATUS_DISCONNECTING 3
#define STATUS_DISCONNECTED 4
#define STATUS_NOHOPE 5

struct mosquitto *g_mosq = NULL;

int mid_sent = -1;
struct mosq_config *cfg = NULL;
struct mosq_user *my_user_config = NULL;

static volatile int status = STATUS_CONNECTING;

int sub_cnt = 0;

void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	UNUSED(mosq);
	UNUSED(obj);
	UNUSED(level);

	// printf("%s\n", str);
}

void my_disconnect_callback(struct mosquitto *mosq, void *obj, int rc, const mosquitto_property *properties)
{
	UNUSED(mosq);
	UNUSED(obj);
	UNUSED(rc);
	UNUSED(properties);

	if(rc == 0){
		status = STATUS_DISCONNECTED;
	}

	printf("Call the function: my_disconnect_callback\n");
}

int my_publish(const char *topic, int payloadlen, void *payload)
{
	struct mosquitto *mosq = g_mosq;
	int mid = 0; 
	int ret = mosquitto_publish_v5(mosq, &mid, topic, payloadlen, payload, 0, true, 0);

	switch(ret){
		case MOSQ_ERR_INVAL:
			printf("Error: Invalid input. Does your topic contain '+' or '#'?\n");
		break;
		case MOSQ_ERR_NOMEM:
			printf("Error: Out of memory when trying to publish message.\n");
		break;
		case MOSQ_ERR_NO_CONN:
			printf("Error: Client not connected when trying to publish.\n");
		break;
		case MOSQ_ERR_PROTOCOL:
			printf("Error: Protocol error when communicating with broker.\n");
		break;
		case MOSQ_ERR_PAYLOAD_SIZE:
			printf("Error: Message payload is too large.\n");
		break;
		case MOSQ_ERR_QOS_NOT_SUPPORTED:
			printf("Error: Message QoS not supported on broker, try a lower QoS.\n");
		break;
	}
	return ret;
}

// user add sub topic
int my_subscribe(const char *sub_topic){
	int mid = 0;
	int qos = 2;
	int topic_count = 1;
	mosquitto_subscribe_multiple(g_mosq, &mid, topic_count, (char *const *const)&sub_topic, qos, 0, NULL);
	return 0;
}

void my_connect_callback(struct mosquitto *mosq, void *obj, int result, int flags, const mosquitto_property *properties)
{
	int rc = MOSQ_ERR_SUCCESS;

	UNUSED(obj);
	UNUSED(flags);
	UNUSED(properties);

   if(!result){
		mosquitto_subscribe_multiple(mosq, NULL, cfg->topic_count, cfg->topics, 2, 0, NULL);
    }else{
        printf("Connect failed\n");
	}

	printf("Call the function: my_connect_callback\n");
}


void my_publish_callback(struct mosquitto *mosq, void *obj, int mid, int reason_code, const mosquitto_property *properties)
{
	char *reason_string = NULL;
	UNUSED(obj);
	UNUSED(properties);
	// printf("Call the function: my_publish_callback\n");
}


// struct mosquitto_message{
// 	int mid;
// 	char *topic;
// 	void *payload;
// 	int payloadlen;
// 	int qos;
// 	bool retain;
// };

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	sub_cnt++;
    if(message->payloadlen){
        // printf(" mosq_client : sub_cnt = %d ---- sub_topic : %s ---- msg : %s \n", sub_cnt, message->topic, (char *)message->payload);
		my_user_config->sub_callback((char *)message->payload, message->payloadlen, message->topic, userdata);
    }else{
        printf("%s (null)\n", message->topic);
    }
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
	return ;
    int i;
    printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
    for(i=1; i<qos_count; i++){
        printf(", %d", granted_qos[i]);
    }
    printf("\n");
}

static void print_version(void)
{
	int major, minor, revision;

	mosquitto_lib_version(&major, &minor, &revision);
	printf("version libmosquitto %d.%d.%d.\n", major, minor, revision);
}

int configure_cfg(struct mosq_user *user_config){
	cfg = (struct mosq_config*)malloc(sizeof(struct mosq_config));
	cfg->quiet = false; // log switch
	cfg->clean_session = true;
	cfg->id_prefix = malloc(strlen(user_config->prog_name)+1);
	if(cfg->id_prefix == NULL){
		return -1;
	}
	memcpy(cfg->id_prefix, user_config->prog_name, strlen(user_config->prog_name)+1);

	cfg->host = "localhost";
	cfg->port = 1883;
	cfg->keepalive = 600;

	cfg->bind_address = NULL;
	cfg->connect_props = NULL;

	cfg->debug = false;

	int i;
	for(i=0;i<user_config->topic_count;i++){
		if(my_cfg_add_sub_topic(cfg, user_config->sub_topics[i]) != 0){
			return 1;
		}
	}

	return 0;
}

// struct mosq_user{
// 	char *prog_name;
// 	char **sub_topics;
// 	int topic_count;
// 	sub_cb sub_callback;
//     exception_cb exception_callback;
//     log_print log_print_callback;
// 	void *userdata;
// };

// main interface()
int start_mosquitto_client(struct mosq_user *user_config)
{
	int rc;

	rc = mosquitto_lib_init();
	if(rc){
		return -1;
	}

	if(configure_cfg(user_config) != 0){
		goto cleanup;
	}
 
	my_user_config = (struct mosq_user*)malloc(sizeof(struct mosq_user));
	my_user_config->sub_callback = user_config->sub_callback;
	my_user_config->exception_callback = user_config->exception_callback;
	my_user_config->log_print_callback = user_config->log_print_callback;
	my_user_config->userdata = user_config->userdata;

	if(client_id_generate(cfg)){
		goto cleanup;
	}

	g_mosq = mosquitto_new(cfg->id, cfg->clean_session, user_config->userdata);
	if(!g_mosq){
		switch(errno){
			case ENOMEM:
				err_printf(cfg, "Error: Out of memory.\n");
				break;
			case EINVAL:
				err_printf(cfg, "Error: Invalid id.\n");
				break;
		}
		goto cleanup;
	}
	if(cfg->debug){
		mosquitto_log_callback_set(g_mosq, my_log_callback);
	}
	mosquitto_connect_v5_callback_set(g_mosq, my_connect_callback);
	mosquitto_disconnect_v5_callback_set(g_mosq, my_disconnect_callback);
	mosquitto_publish_v5_callback_set(g_mosq, my_publish_callback);
    mosquitto_message_callback_set(g_mosq, my_message_callback);
    mosquitto_subscribe_callback_set(g_mosq, my_subscribe_callback);

	// if(client_opts_set(mosq, cfg)){
	// 	goto cleanup;
	// }
	rc = mosquitto_connect_bind_v5(g_mosq, cfg->host, cfg->port, cfg->keepalive, cfg->bind_address, cfg->connect_props);
	if(rc){
		goto cleanup;
	}

	rc = mosquitto_loop_start(g_mosq);
	if(rc != MOSQ_ERR_SUCCESS)
	{
		printf("mosquitto loop error\n");
		goto cleanup;
	}

	return 0;

cleanup:
	mosquitto_lib_cleanup();
	client_config_cleanup(cfg);
	return 1;
}

void stop_mosquitto_client(){
	mosquitto_loop_stop(g_mosq, false);
	mosquitto_destroy(g_mosq);
	mosquitto_lib_cleanup();
	// client_config_cleanup(&cfg);
}

