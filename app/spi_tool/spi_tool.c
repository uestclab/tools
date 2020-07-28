/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include "tool.h"
#include <linux/spi/spidev.h>
#include "cJSON.h"
#include "read_file.h"
#include "str2digit.h"

#define BUFF_LEN 256

uint8_t default_rx[BUFF_LEN] = {0, };
uint8_t padd_tx[BUFF_LEN] = {0, };

static uint32_t mode;
static uint8_t bits = 8;
static uint32_t speed = 0x7A120;

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static void transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
	int ret;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	if (mode & SPI_TX_QUAD){
		printf("mode & SPI_TX_QUAD \n");
		tr.tx_nbits = 4;
	}
	else if (mode & SPI_TX_DUAL){
		printf("mode & SPI_TX_DUAL \n");
		tr.tx_nbits = 2;
	}
	if (mode & SPI_RX_QUAD){
		printf("mode & SPI_RX_QUAD \n");
		tr.rx_nbits = 4;
	}
	else if (mode & SPI_RX_DUAL){
		printf("mode & SPI_RX_DUAL \n");
		tr.rx_nbits = 2;
	}
	if (!(mode & SPI_LOOP)) {
		if (mode & (SPI_TX_QUAD | SPI_TX_DUAL)){
			printf("mode & (SPI_TX_QUAD | SPI_TX_DUAL \n");
			tr.rx_buf = 0;
		}
		else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL)){
			printf("mode & (SPI_RX_QUAD | SPI_RX_DUAL \n");
			tr.tx_buf = 0;
		}
	}

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

}

void get_spidev(char* buf, char** dev){
	cJSON * root = NULL;
    cJSON * item = NULL;
    root = cJSON_Parse(buf);
	item = cJSON_GetObjectItem(root , "spidev");
	*dev = (char*)xzalloc(strlen(item->valuestring)+1);
	memcpy(*dev, item->valuestring, strlen(item->valuestring)+1);
	cJSON_Delete(root);
}

int process_spi_json(int fd, char* buf){

	printf("process_spi_json \n");

	int instuction;
	cJSON * root = NULL;
    cJSON * item = NULL;
	cJSON * array_item = NULL;
    root = cJSON_Parse(buf);


	array_item = cJSON_GetObjectItem(root , "op_cmd");

	int op_cnt = cJSON_GetArraySize(array_item);

	printf("op_cnt = %d \n", op_cnt);

	for(int i=0;i<op_cnt;i++){
		cJSON * pSub = cJSON_GetArrayItem(array_item, i);
		item = cJSON_GetObjectItem(pSub,"instruction_data");
		// item->valuestring
		instuction = bb_strtoull(item->valuestring, NULL, 16);
		instuction = htonl(instuction) >> 8;
		transfer(fd, (char*)(&instuction), default_rx, 3);
		printf("instruction_data : %s \n", item->valuestring);
	}
	cJSON_Delete(root);

	return 0;
}

int main(int argc, char *argv[])
{
	int instuction;
	char *device = NULL; //"/dev/spidev1.0";
    char* jsonBuf = NULL;

	if(argc == 3){
		device = argv[1];
		instuction = bb_strtoull(argv[2], NULL, 16);
		instuction = htonl(instuction) >> 8;
		printf("device : %s , instuction : %s , 0x%x \n", device, argv[2], instuction);
    }else if(argc == 2){
		jsonBuf = get_json_buf(argv[1]);
		printf("get json : %s \n", jsonBuf);
		get_spidev(jsonBuf,&device);

		printf("spidev : %s \n", device);
	}else{
		printf("usage : spi_tool [spidev] [instuction] or spi_tool [json]\n");
		return 0;
	}
	/* spi init */

	int ret = 0;
	int fd;

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

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


	if(jsonBuf == NULL){
		transfer(fd, (char*)(&instuction), default_rx, 3);
		printf("running instruction_1\n");
		char ret_str[16];

		snprintf(ret_str, ARRAY_SIZE(ret_str), "0x%x", default_rx[2]);

		printf("return value : %s \n", ret_str);
		
		//sleep(10);
	}else{
		process_spi_json(fd, jsonBuf);

		printf("spi transfer end !\n");
	}


	close(fd);



	return ret;
}
