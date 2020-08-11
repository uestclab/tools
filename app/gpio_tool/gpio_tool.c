#include "tool.h"
#include "cJSON.h"
#include "read_file.h"
#include "str2digit.h"
#include "gpio_cmd.h"
#include "gpio_dev.h"
#include "gpio_ops.h"

#pragma pack(1)
struct gpio_op_cmd_s{
	char name[128];
	int  id;
	int intr;
	int act;
	int gpio_no;
	int dir;
	int val;
#define	UNSUPPORT	-1
	int excep_val;
	int waite_time;//us
};
#pragma pack()

typedef struct gpio_info_t{
	struct list_head headNode;
	int item_num;
}gpio_info_t;

typedef struct gpio_node_t{
	struct list_head list;
	int fd;
	int read_expect;
	struct gpio_op_cmd_s cmd;
}gpio_node_t;


void init_gpio_info(gpio_info_t** handler){
	*handler = (gpio_info_t*)xzalloc(sizeof(gpio_info_t));
	gpio_info_t* gpio_handler = *handler;
	gpio_handler->item_num = 0;
	INIT_LIST_HEAD(&(gpio_handler->headNode));
}

int parse_gpio(char* buf, gpio_info_t* handler){
	cJSON * root = NULL;
    cJSON * item = NULL;
    root = cJSON_Parse(buf);

	cJSON * array_item = NULL;

	array_item = cJSON_GetObjectItem(root , "op_cmd");

	int op_cnt = cJSON_GetArraySize(array_item);
	handler->item_num = op_cnt;
	printf("op_cnt = %d \n", op_cnt);

	for(int i=0;i<op_cnt;i++){
		gpio_node_t* node = (gpio_node_t*)xzalloc(sizeof(gpio_node_t));
		cJSON * pSub = cJSON_GetArrayItem(array_item, i);
		item = cJSON_GetObjectItem(pSub,"gpio_no");
		node->cmd.gpio_no = get_item_val(item->valuestring);

		if(cJSON_HasObjectItem(pSub,"name") == 1){
			item = cJSON_GetObjectItem(pSub , "name");
			memcpy(node->cmd.name, item->valuestring, strlen(item->valuestring)+1);
		}

		if(cJSON_HasObjectItem(pSub,"id") == 1){
			item = cJSON_GetObjectItem(pSub , "id");
			node->cmd.id = get_item_val(item->valuestring);
		}

		if(cJSON_HasObjectItem(pSub,"intr") == 1){
			item = cJSON_GetObjectItem(pSub , "intr");
			node->cmd.intr = get_item_val(item->valuestring);
		}

		if(cJSON_HasObjectItem(pSub,"dir") == 1){
			item = cJSON_GetObjectItem(pSub , "dir");
			node->cmd.dir = get_item_val(item->valuestring);
		}

		if(cJSON_HasObjectItem(pSub,"val") == 1){
			item = cJSON_GetObjectItem(pSub , "val");
			node->cmd.val = get_item_val(item->valuestring);
		}

		if(cJSON_HasObjectItem(pSub,"exp") == 1){
			item = cJSON_GetObjectItem(pSub , "exp");
			node->cmd.excep_val = get_item_val(item->valuestring);
			node->read_expect = 1;
		}

		if(cJSON_HasObjectItem(pSub,"sleep") == 1){
			item = cJSON_GetObjectItem(pSub , "sleep");
			node->cmd.waite_time = get_item_val(item->valuestring);
		}

		list_add_tail(&(node->list), &(handler->headNode));
	}
	
	cJSON_Delete(root);
	return 0;
}

int process_gpio_cmd(gpio_info_t* handler){

	printf("process_gpio_cmd()\n");

	int ret = 0;
	struct list_head *pos = NULL;
	gpio_node_t* tmp_node = NULL;
	int cnt = handler->item_num;
	struct gpio_op_cmd_s* tmp_cmd = NULL; 
	list_for_each(pos, &(handler->headNode)){
		tmp_node = list_entry(pos, struct gpio_node_t, list);
		tmp_cmd = &(tmp_node->cmd);
		// printf("name = %s, id = %d, intr = %d, act = %d, gpio_no = %d, dir = %d, val = %d, excep_val = %d, waite_time = %d \n", 
		// 		tmp_cmd->name, tmp_cmd->id, tmp_cmd->intr, tmp_cmd->act, tmp_cmd->gpio_no, 
		// 		tmp_cmd->dir, tmp_cmd->val, tmp_cmd->excep_val, tmp_cmd->waite_time);

		if(gpio_open(tmp_cmd->gpio_no, tmp_cmd->dir) < 0){
			return -1;
		}

		if(GPIO_DIR_OUT == tmp_cmd->dir){
			// write
			if(gpio_set_val(tmp_cmd->gpio_no, tmp_cmd->val) < 0){
				printf("write failed : gpio_no = %d , val = %d \n", tmp_cmd->gpio_no, tmp_cmd->val);
			}else{
				printf("write success : gpio_no = %d , val = %d \n", tmp_cmd->gpio_no, tmp_cmd->val);
			}

			if(tmp_cmd->waite_time){
				usleep(tmp_cmd->waite_time);
			}

		}else{
			// read
			int8_t read_val = -1;
			if(gpio_get_val(tmp_cmd->gpio_no, &read_val) < 0 ){
				printf("read failed !\n");
			}else{
				if(tmp_node->read_expect){
					if(tmp_cmd->excep_val == read_val){
						printf("read == expect: gpio_no = %d , read_val = %d, expect = %d \n", 
							tmp_cmd->gpio_no, read_val, tmp_cmd->excep_val);
					}else{
						printf("read != expect: gpio_no = %d , read_val = %d, expect = %d \n", 
							tmp_cmd->gpio_no, read_val, tmp_cmd->excep_val);						
					}
				}else{
					printf("read : gpio_no = %d , read_val = %d \n", tmp_cmd->gpio_no, read_val);
				}
			}
		}

	}

	return 0;
}

int main(int argc, char *argv[])
{

	if(argc < 2){
		return 0;
	}

	gpio_info_t *gpio_handler;
	init_gpio_info(&gpio_handler);

	g_args_para g_args;
	if(parse_cmd_line(argc, argv, &g_args)){
		return 0;
	}

	char* jsonBuf = get_json_buf(g_args.json_path);
	printf("get json : %s \n", jsonBuf);

	if(parse_gpio(jsonBuf,gpio_handler) == -1){
		return 0;
	}

	process_gpio_cmd(gpio_handler);

	return 0;
}
