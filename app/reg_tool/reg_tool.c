
#include "tool.h"
#include "cJSON.h"
#include "read_file.h"
#include "str2digit.h"

struct reg_op_cmd_s{
	unsigned char cmd; //0--read  1--write
	int value;
	int offset;
	int waite_time; // us
};

typedef struct reg_info_t{
	int base;
	int cnt;
	struct reg_op_cmd_s *c;
}reg_info_t;

char* run_read_cmd(int addr)
{
    FILE* fp = NULL; 
    char* value = NULL; 
    int BUFSZ = 100; 
    char buf[BUFSZ]; 
    char command[150]; 
 
    sprintf(command, "devmem 0x%x", addr);
	//printf("command : %s\n",command); 

    if((fp = popen(command,"r")) == NULL) 
    { 
        return NULL;
    } 
    if((fgets(buf,BUFSZ,fp))!= NULL) 
    {
		//printf("buf:%s\n",buf);
		value = (char*)malloc(strlen(buf)+1);
        memcpy(value,buf,strlen(buf)+1); 
    }
    
    pclose(fp); 
    
    fp=NULL; 
    
	return value;
}

void init_reg_info(reg_info_t* reg_info){
	reg_info->base = 0;
	reg_info->cnt  = 0;
	if(reg_info->c){
		free(reg_info->c);
		reg_info->c = NULL;
	}
}

int process_reg_json(reg_info_t* reg_info, char* buf){

	printf("process_reg_json \n");

	char cmd[64];

	init_reg_info(reg_info);

	cJSON * root = NULL;
    cJSON * item = NULL;
	cJSON * array_item = NULL;
    root = cJSON_Parse(buf);

	if(cJSON_HasObjectItem(root,"base") == 0){
		printf("invalid reg json !\n");
		return -1;
	}
	item = cJSON_GetObjectItem(root , "base");
	reg_info->base = bb_strtoull(item->valuestring, NULL, 16);

	array_item = cJSON_GetObjectItem(root , "op_cmd");

	int op_cnt = cJSON_GetArraySize(array_item);

	reg_info->cnt = op_cnt;

	reg_info->c = (struct reg_op_cmd_s*)xzalloc(sizeof(struct reg_op_cmd_s) * op_cnt);

	printf("op_cnt = %d \n", op_cnt);

	for(int i=0;i<op_cnt;i++){
		cJSON * pSub = cJSON_GetArrayItem(array_item, i);

		if(cJSON_HasObjectItem(pSub,"offset") == 1){
			item = cJSON_GetObjectItem(pSub,"offset");
			reg_info->c[i].offset = bb_strtoull(item->valuestring, NULL, 16);
		}


		if(cJSON_HasObjectItem(pSub,"cmd") == 1){
			item = cJSON_GetObjectItem(pSub,"cmd");
			reg_info->c[i].cmd = bb_strtoull(item->valuestring, NULL, 10);
		}

		if(cJSON_HasObjectItem(pSub,"val") == 1){
			item = cJSON_GetObjectItem(pSub,"val");
			reg_info->c[i].value = bb_strtoull(item->valuestring, NULL, 16);
		}

		if(cJSON_HasObjectItem(pSub,"waite_time") == 1){
			item = cJSON_GetObjectItem(pSub,"waite_time");
			reg_info->c[i].waite_time = bb_strtoull(item->valuestring, NULL, 10);
		}

		//devmem 0x43c20100 32 0xcf000fff

		int addr = reg_info->c[i].offset + reg_info->base;

		if(reg_info->c[i].cmd){
			sprintf(cmd, "devmem 0x%x 32 0x%x", addr, reg_info->c[i].value);
			system(cmd);
			printf("write: %s \n", cmd);
			if(reg_info->c[i].waite_time !=0){
				printf("usleep(%d)\n",reg_info->c[i].waite_time);
				usleep(reg_info->c[i].waite_time);
			}

		}else{
			char* ret = run_read_cmd(addr);
			if(ret){
				printf("read: devmem 0x%x --- ret = %s\n", addr, ret);
				free(ret);
			}
		}

	}
	cJSON_Delete(root);

	free(reg_info->c);

	return 0;
}

int main(int argc, char *argv[])
{
    char* jsonBuf = NULL;
	reg_info_t reg_info;

	if(argc == 2){
		jsonBuf = get_json_buf(argv[1]);
		printf("get json : %s \n", jsonBuf);
	}else{
		printf("usage : reg_tool [json_path]\n");
		return 0;
	}

	int ret = process_reg_json(&reg_info, jsonBuf);

	printf("reg_tool end !\n");

	return ret;
}

