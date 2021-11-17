
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include "aagc_ops.h"
#include "gw_mosquitto/mosquitto_client.h"
#include "cJSON.h"

char* rssi_data_json(float rssi_data, int seq_num){
    char* rssi_data_json = NULL;
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "comment", "aagc_rssi_data");
    cJSON_AddNumberToObject(root, "seq_num", seq_num);
    cJSON_AddNumberToObject(root, "rssi", rssi_data);

    rssi_data_json = cJSON_Print(root);
    cJSON_Delete(root);
    return rssi_data_json;
}



void *do_work(void *arg) {
    struct aagc_state *p_state = arg;
    const char *topic = "gw/rssi_topic";
    int seq_num = 0;
    while(1){
        float rssi = get_rssi(p_state);

        char* rssi_json = rssi_data_json(rssi,seq_num);
        int ret = my_publish(topic, strlen(rssi_json)+1, rssi_json);
        // printf("rssi_json = %s , \n ret = %d \n", rssi_json , ret);
        seq_num++;
        free(rssi_json);
        usleep(1000000);
    }

    return 0;
}


int start_publish_rssi(struct aagc_state *p_state){
    pthread_t tid;
    if(pthread_create(&tid, NULL, do_work, (void*)(p_state)) != 0){
        return -1;
    }
    return 0;
}

