
#include "tool.h"
#include <linux/spi/spidev.h>
#include "cJSON.h"
#include "read_file.h"
#include "str2digit.h"
#include "spi_cmd.h"



uint16_t run_i2cget_cmd(int addr, int reg, int w_flag)
{
    FILE* fp = NULL; 
    char* value = NULL; 
    int BUFSZ = 100; 
    char buf[BUFSZ]; 
    char command[150]; 
 
	if(w_flag){
		sprintf(command, "i2cget -f -y 0 0x%x 0x%x w", addr, reg);
	}else{
		sprintf(command, "i2cget -f -y 0 0x%x 0x%x", addr, reg);
	}
	//printf("command : %s\n",command); 

    if((fp = popen(command,"r")) == NULL) 
    { 
        return 0;
    } 
    if((fgets(buf,BUFSZ,fp))!= NULL) 
    {
		value = (char*)malloc(strlen(buf)+1);
        memcpy(value,buf,strlen(buf)+1);
		printf("get value : %s", value); 
    }
    
    pclose(fp); 
    
    fp=NULL;

	uint16_t ret = bb_strtoull(value, NULL, 16);
	return ret;

}

void run_i2cset_cmd(int addr, int reg, uint8_t value)
{
    char command[128];
    sprintf(command, "i2cset -f -y 0 0x%x 0x%x 0x%x", addr, reg, value);
	system(command);
}

// ad7417 地址选择
void run_i2cset_swiich_cmd(int addr, int switch_value)
{
    char command[128];
    sprintf(command, "i2cset -f -y 0 0x%x 0x%x", addr, switch_value);
	system(command);
}


/* ----  get operation ---- */

// 整板温度
double get_board_temper(){

// i2cset -y -f 0 0x28 0x00 (地址选择)
// i2cset -y -f 0 0x28 0x01 0x00
//    #等待1ms
// i2cget -y -f 0 0x28 0x00  w   #连续输出两个字节
	double temper = 0;
	run_i2cset_swiich_cmd(0x28,0x00);
	run_i2cset_cmd(0x28,0x01,0x00);
	usleep(1000);

	uint8_t reg = 0x00;
	uint16_t val = run_i2cget_cmd(0x28,reg,1);

	printf("temper : 0x%x\n", val);

	uint16_t place0 = val & (0xff);
	uint16_t place1 = (val & (0xff00)) >> 8;

	uint16_t tmp = (place0 << 2) | (place1 >> 6);
	tmp = tmp & 0x3ff;
	int msb = tmp & 0x2ff;
	if(msb){ // Negative
		temper = (tmp & 0x1ff - 512) * 1.0 / 4;
	}else{
		temper = (tmp) * 1.0 / 4;
	}

    return temper;
}



#define BUFF_LEN 256

uint8_t default_rx[BUFF_LEN] = {0, };
uint8_t padd_tx[BUFF_LEN] = {0, };

// static uint32_t mode;
// static uint8_t bits = 8;
// static uint32_t speed = 0x7A120;


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
	int fd;
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
	spi_handler->fd  = -1;
	spi_handler->spimode = 0;
	spi_handler->spimaxclk = 0x7A120;
	spi_handler->spibpw = 8;
	spi_handler->cnt = 0;
	if(spi_handler->c){
		free(spi_handler->c);
		spi_handler->c = NULL;
	}
}

static void pabort(const char *s)
{
	perror(s);
	abort();
}

int init_spidev(spi_info_t* spi_handler){

	char *device = spi_handler->spidev;

	uint32_t mode = spi_handler->spimode;
	uint8_t bits = spi_handler->spibpw;
	uint32_t speed = spi_handler->spimaxclk;	

	int ret = 0;
	int fd;

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	spi_handler->fd = fd;

	mode = mode | SPI_MODE_0;
	printf("spi mode: 0x%x\n", mode);
	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: 0x%x\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
}

void close_spidev(spi_info_t* spi_handler){
	close(spi_handler->fd);
	if(spi_handler->spidev){
		free(spi_handler->spidev);
	}
	if(spi_handler->c){
		free(spi_handler->c);
	}
}

static void transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len, uint32_t speed, uint8_t bits)
{
	int ret;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

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
			transfer(spi_handler->fd, (char*)(&instuction), default_rx, 3, spi_handler->spimaxclk, spi_handler->spibpw);
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
				transfer(spi_handler->fd, (char*)(&instuction), default_rx, 3, spi_handler->spimaxclk, spi_handler->spibpw);
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
	printf("spi success \n");
	return 0;
}

int main(int argc, char *argv[])
{
	spi_info_t *spi_handler;
	init_spi_info(&spi_handler);

	int ret;
	int instuction;
    char* jsonBuf = NULL;
/* for test cmd */
	// g_args_para g_args;
	// if(parse_cmd_line(argc, argv, &g_args)){
	// 	return 0;
	// }else{
	// 	printf("spidev : %s , json_path : %s, instruction : %u\n", g_args.spidev, g_args.json_path, g_args.instuction);
	// 	return 0;
	// }
/* ------------ */
	if(argc == 3){ //spi_tool [spidev] [instuction]
		spi_handler->spidev = malloc(strlen(argv[1])+1);
		memcpy(spi_handler->spidev,argv[1],strlen(argv[1])+1);
		instuction = bb_strtoull(argv[2], NULL, 16);
		instuction = htonl(instuction) >> 8;
		printf("device : %s , instuction : %s , 0x%x \n", spi_handler->spidev, argv[2], instuction);
    }else if(argc == 2){ //spi_tool [json]
		jsonBuf = get_json_buf(argv[1]);
		printf("get json : %s \n", jsonBuf);
		if(parse_spidev(jsonBuf,spi_handler) == -1){
			return 0;
		}
		printf("spidev : %s \n", spi_handler->spidev);
	}else{
		printf("usage : spi_tool [spidev] [instuction] or spi_tool [json]\n");
		return 0;
	}
	/* spi init */
	ret = init_spidev(spi_handler);

	if(jsonBuf == NULL){ //spi_tool [spidev] [instuction]
		transfer(spi_handler->fd, (char*)(&instuction), default_rx, 3, spi_handler->spimaxclk, spi_handler->spibpw);
		printf("running instruction_1\n");
		char ret_str[16];
		snprintf(ret_str, ARRAY_SIZE(ret_str), "0x%02X%02X%02X", default_rx[0], default_rx[1], default_rx[2]);
		printf("rx: 0x%x , 0x%x , 0x%x \n", default_rx[0], default_rx[1], default_rx[2]);
		printf("return value : %s \n", ret_str);
	}else{ //spi_tool [json]
		process_spi_cmd(spi_handler);
		printf("spi transfer end !\n");
		free(jsonBuf);
	}


	close_spidev(spi_handler);

	return 0;
}
