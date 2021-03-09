
#include "tool.h"
#include "str2digit.h"
#include "spi_gpio_ops.h"
#include "cmd_line.h"
#include "cJSON.h"
#include "read_file.h"

#define BUFF_LEN 3

uint8_t default_rx[BUFF_LEN] = {0, };

struct spi_op_cmd_s{
	unsigned char cmd; //0--read  1--write
	uint32_t instuction;

	int			  ifcon1;
	unsigned char ifcon_flag1; 
	uint32_t	  ifcon_mask1;

	int			  ifcon2;
	unsigned char ifcon_flag2; 
	uint32_t	  ifcon_mask2;

	int			  waite_time_ifcon;
	int           waite_time; // us
};

typedef struct spi_info_t{
	char *spidev;
	uint32_t spimode;
	uint32_t spimaxclk; //speed
	uint8_t spibpw; // bits
	int cnt;
	struct spi_op_cmd_s *c;
}spi_info_t;

void init_spi_info(spi_info_t** handler){

	*handler = (spi_info_t*)xzalloc(sizeof(spi_info_t));

	spi_info_t* spi_handler = *handler;

	spi_handler->spidev = NULL;
	spi_handler->spimode = 1;
	spi_handler->spimaxclk = 0x3E8;
	spi_handler->spibpw = 8;
	spi_handler->cnt = 0;
	if(spi_handler->c){
		free(spi_handler->c);
		spi_handler->c = NULL;
	}
}

int parse_spidev(char* buf, spi_info_t* spi_handler){
	cJSON * root = NULL;
    cJSON * item = NULL;
    root = cJSON_Parse(buf);

	if(cJSON_HasObjectItem(root,"spidev") == 0){
		printf("invalid spi json !\n");
		return -1;
	}
	item = cJSON_GetObjectItem(root , "spidev");
	spi_handler->spidev = (char*)xzalloc(strlen(item->valuestring)+1);
	memcpy(spi_handler->spidev, item->valuestring, strlen(item->valuestring)+1);

	if(cJSON_HasObjectItem(root,"spimode") == 1){
		item = cJSON_GetObjectItem(root , "spimode");
		spi_handler->spimode = bb_strtoull(item->valuestring, NULL, 16);
	}
	if(cJSON_HasObjectItem(root,"spimaxclk") == 1){
		item = cJSON_GetObjectItem(root , "spimaxclk");
		spi_handler->spimaxclk = bb_strtoull(item->valuestring, NULL, 16);
	}
	if(cJSON_HasObjectItem(root,"spibpw") == 1){
		item = cJSON_GetObjectItem(root , "spibpw");
		spi_handler->spibpw = bb_strtoull(item->valuestring, NULL, 16);
	}

	cJSON * array_item = NULL;

	array_item = cJSON_GetObjectItem(root , "op_cmd");

	int op_cnt = cJSON_GetArraySize(array_item);
	spi_handler->cnt = op_cnt;
	printf("op_cnt = %d \n", op_cnt);

	spi_handler->c = (struct spi_op_cmd_s*)xzalloc(sizeof(struct spi_op_cmd_s) * op_cnt);

	for(int i=0;i<op_cnt;i++){
		cJSON * pSub = cJSON_GetArrayItem(array_item, i);
		item = cJSON_GetObjectItem(pSub,"instruction_data");
		int instuction;
		instuction = bb_strtoull(item->valuestring, NULL, 16);
		spi_handler->c[i].instuction = htonl(instuction) >> 8;

		item = cJSON_GetObjectItem(pSub,"cmd");
		spi_handler->c[i].cmd = bb_strtoull(item->valuestring, NULL, 16);

		if(cJSON_HasObjectItem(pSub,"ifcon1") == 1){
			item = cJSON_GetObjectItem(pSub , "ifcon1");
			spi_handler->c[i].ifcon1 = bb_strtoull(item->valuestring, NULL, 16);
		}

		if(cJSON_HasObjectItem(pSub,"ifcon_flag1") == 1){
			item = cJSON_GetObjectItem(pSub , "ifcon_flag1");
			spi_handler->c[i].ifcon_flag1 = bb_strtoull(item->valuestring, NULL, 16);
		}

		if(cJSON_HasObjectItem(pSub,"ifcon_mask1") == 1){
			item = cJSON_GetObjectItem(pSub , "ifcon_mask1");
			spi_handler->c[i].ifcon_mask1 = bb_strtoull(item->valuestring, NULL, 16);
		}

		if(cJSON_HasObjectItem(pSub,"ifcon2") == 1){
			item = cJSON_GetObjectItem(pSub , "ifcon2");
			spi_handler->c[i].ifcon2 = bb_strtoull(item->valuestring, NULL, 16);
		}

		if(cJSON_HasObjectItem(pSub,"ifcon_flag2") == 1){
			item = cJSON_GetObjectItem(pSub , "ifcon_flag2");
			spi_handler->c[i].ifcon_flag2 = bb_strtoull(item->valuestring, NULL, 16);
		}

		if(cJSON_HasObjectItem(pSub,"ifcon_mask2") == 1){
			item = cJSON_GetObjectItem(pSub , "ifcon_mask2");
			spi_handler->c[i].ifcon_mask2 = bb_strtoull(item->valuestring, NULL, 16);
		}

		if(spi_handler->c[i].ifcon_flag1 || spi_handler->c[i].ifcon_flag2){
				spi_handler->c[i].waite_time_ifcon = 1000000;
				spi_handler->c[i].waite_time = 0;
		}

		if(cJSON_HasObjectItem(pSub,"waite_time_ifcon") == 1){
			item = cJSON_GetObjectItem(pSub , "waite_time_ifcon");
			spi_handler->c[i].waite_time_ifcon = bb_strtoull(item->valuestring, NULL, 16);
		}

		if(cJSON_HasObjectItem(pSub,"waite_time") == 1){
			item = cJSON_GetObjectItem(pSub , "waite_time");
			spi_handler->c[i].waite_time = get_item_val(item->valuestring);
		}

	}
	
	cJSON_Delete(root);
	return 0;
}

int init_spidev(spi_info_t* spi_handler){

	uint32_t mode = spi_handler->spimode;
	uint8_t bits = spi_handler->spibpw;
	uint32_t speed = spi_handler->spimaxclk;

	int spi_delay_time_us = (int)((1.0 / speed) * 1000 * 1000);

	if(simulate_spi_init(spi_delay_time_us, mode) < 0){
		printf("spi gpio init failed ! \n");
		return 0;
	}	

	printf("spi mode: 0x%x\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
	printf("spi_delay_time_us: %d us\n", spi_delay_time_us);
}

void close_spidev(spi_info_t* spi_handler){

	if(spi_handler->spidev){
		free(spi_handler->spidev);
	}
	if(spi_handler->c){
		free(spi_handler->c);
	}
}

void write_to_5682(int instuction){
	spi_write((char*)(&instuction), 3);
}

int process_spi_cmd(spi_info_t* spi_handler){

	uint32_t instuction;
	int con1 = 1;
	int con2 = 1;
	struct timeval start_time;
	int op_cnt = spi_handler->cnt;

	int state = 0;

	for(int i=0;i<op_cnt;i++){
		if(spi_handler->c[i].cmd){
			instuction = spi_handler->c[i].instuction;
			write_to_5682(instuction);
			//printf("instruction_data : 0x%x \n", htonl(instuction)>>8);
			if(spi_handler->c[i].waite_time){
				usleep(spi_handler->c[i].waite_time);
			}	
		}else{
			con1 = 1;
			con2 = 1;
			get_usec(&start_time);
			for(;;){
				instuction = spi_handler->c[i].instuction;
				//transfer(spi_handler->fd, (char*)(&instuction), default_rx, 3, spi_handler->spimaxclk, spi_handler->spibpw);
				printf("for: instruction_data 0x%x\n", htonl(instuction)>>8);
				char ret_str[16];
				snprintf(ret_str, ARRAY_SIZE(ret_str), "0x%02X%02X%02X", default_rx[0], default_rx[1], default_rx[2]);
				printf("read cmd return value : %s \n", ret_str);
				if(spi_handler->c[i].ifcon_flag1){
					con1 = (((default_rx[2] & spi_handler->c[i].ifcon_mask1) == spi_handler->c[i].ifcon1) ) ;
				}

				if(spi_handler->c[i].ifcon_flag2){
					con2 = (((default_rx[2] & spi_handler->c[i].ifcon_mask2) == spi_handler->c[i].ifcon2) ) ;
					printf("con2 = %d --- ret : 0x%x , mask : 0x%x , ifcon2 : 0x%x\n", con2, 
					default_rx[2], spi_handler->c[i].ifcon_mask2, spi_handler->c[i].ifcon2);
				}

				if(spi_handler->c[i].waite_time){
					usleep(spi_handler->c[i].waite_time);
				}

				if(con1 && con2){
					break;
				}else{
					if(time_expire(start_time, spi_handler->c[i].waite_time_ifcon)){
						printf("read state error -- instruction_data 0x%x\n", htonl(instuction)>>8);
						state = 1;
						break;
					}
				}
			}
		}
		if(state){
			return -1;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{

	spi_info_t *spi_handler;
	init_spi_info(&spi_handler);

	int ret;
	int instuction;
    char* jsonBuf = NULL;

	if(argc == 3){ //5682_spi_tool spi_simulate [instuction]
		spi_handler->spidev = malloc(strlen(argv[1])+1);
		memcpy(spi_handler->spidev,argv[1],strlen(argv[1])+1);
		instuction = bb_strtoull(argv[2], NULL, 16);
		instuction = htonl(instuction) >> 8;
		printf("device : %s , instuction : %s , 0x%x \n", spi_handler->spidev, argv[2], instuction);
    }else if(argc == 2){ //5682_spi_tool [json]
		jsonBuf = get_json_buf(argv[1]);
		printf("get json : %s \n", jsonBuf);
		if(parse_spidev(jsonBuf,spi_handler) == -1){
			return 0;
		}
		printf("spidev : %s \n", spi_handler->spidev);
	}else{
		printf("usage : 5682_spi_tool spi_simulate [instuction] or 5682_spi_tool [json]\n");
		return 0;
	}

	/* spi init */
	ret = init_spidev(spi_handler);

	if(jsonBuf == NULL){ //5682_spi_tool spi_simulate [instuction]
		write_to_5682(instuction);
	}else{ //5682_spi_tool [json]
		process_spi_cmd(spi_handler);
		free(jsonBuf);
	}

	close_spidev(spi_handler);

	printf(" complete spi write \n");

	return 0;
}

