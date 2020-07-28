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

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/time.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include "cJSON.h"

#define BUFF_LEN 256

uint8_t default_rx[BUFF_LEN] = {0, };
uint8_t padd_tx[BUFF_LEN] = {0, };

static uint32_t mode;
static uint8_t bits = 8;
static uint32_t speed = 0x7A120;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static inline void* xmalloc(size_t size)
{
	void *ptr = malloc(size);
	return ptr;
}

static inline void* xzalloc(size_t size)
{
	void *ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

static inline void* xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	return ptr;
}

#define	xfree(ptr) do{ \
	if(ptr){ \
		free(ptr); \
		ptr = NULL; \
	} \
}while(0)

int read_config_file(const char *path, char **file_buf)
{
	int fd, ret;
	struct stat buf;

	if(NULL == path){
		ret = -EINVAL;
		goto exit;
	}
	
	ret = fd =  open(path,(O_RDONLY | O_SYNC), 0666);
	if(ret < 0){
		//log_error("open(%s)err",path);
		goto exit;
	}
	fstat(fd, &buf);
	
	*file_buf = (char*)xzalloc(buf.st_size + 1); //�����һ���ֽ�
	if(NULL == *file_buf){
		//log_error("malloc(%s)err",path);
		ret = -ENOMEM;
		goto exit;
	}
	
	ret = read(fd,*file_buf,buf.st_size);
	close(fd);
exit:
	return ret;
}

static unsigned long long ret_ERANGE(void)
{
	errno = ERANGE; /* this ain't as small as it looks (on glibc) */
	return 0;//ULLONG_MAX;
}

static int bb_ascii_isalnum(unsigned char a)
{
	unsigned char b = a - '0';
	if (b <= 9)
		return (b <= 9);
	b = (a|0x20) - 'a';
	return b <= 'z' - 'a';
}

static unsigned long long handle_errors(unsigned long long v, char **endp)
{
	char next_ch = **endp;

	/* errno is already set to ERANGE by strtoXXX if value overflowed */
	if (next_ch) {
		/* "1234abcg" or out-of-range? */
		if (bb_ascii_isalnum(next_ch) || errno)
			return ret_ERANGE();
		/* good number, just suspicious terminator */
		errno = EINVAL;
	}
	return v;
}


int bb_strtoull(const char *arg, char **endp, int base)
{
	unsigned long long v;
	char *endptr;
	int index = 0;

	if (!endp) endp = &endptr;
	*endp = (char*) arg;

	/* strtoul("  -4200000000") returns 94967296, errno 0 (!) */
	/* I don't think that this is right. Preventing this... */
	if (!bb_ascii_isalnum(arg[0])){
		if('-' == arg[0]){
			index = 1;
		}else{
			return ret_ERANGE();
		}
	}

	/* not 100% correct for lib func, but convenient for the caller */
	errno = 0;
	v = strtoull(&arg[index], endp, base);
	v = handle_errors(v, endp);
	return index?-v:v;
}

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

char* get_json_buf(char* path){
    char *file_buf;

    int ret = read_config_file(path, &file_buf);

	if(ret > 0){
		return file_buf;
	}else{
		return NULL;
	}
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
