#ifndef	__SPI_COMMON_H__
#define	__SPI_COMMON_H__
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netdb.h>
#include <setjmp.h>
#include <signal.h>
#include <paths.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>


#include <linux/types.h>
#include <linux/spi/spidev.h>

#define	CF_DEV	0
#define	CT_DEV	1
#define	CL_DEV	2

struct spidev_para_s{
	char spidev[256];
	int spimode;
	int spimaxclk; 
	int spibpw;

	void *priv;
};

struct spi_dev_s{
	int fd;

	struct spi_ioc_transfer tr;

	struct	spidev_para_s	*dev_use; // poit to cl_dev cf_dev ct_dev
	
	struct	spidev_para_s	cl_dev;
	struct	spidev_para_s	cf_dev;
	struct	spidev_para_s	ct_dev;
};


#define	LONG_SPI_DEV_OPT\
	{ "spidev",  1, NULL, 'i' },\
	{ "mode",    1, NULL, 'm' },\
	{ "clk",     1, NULL, 'c' },\
	{ "bpw",     1, NULL, 'b' }

#define	SHORT_SPI_DEV_OPT	"i:m:c:b:"


#define	SPI_DEV_HELP_INFO\
	" -i --spidev\t\tspi device path(eg. /dev/spidev1.0 )\n"\
	" -m --mode\t\tspi mode(0\1\2\3)\n"\
	" -c --clk\t\tspi max clk(Hz)\n"\
	" -b --bpw\t\tspi bit perword\n"

void spidev_init(void **dev);

void spidev_set_user_conf(void* dev, int use);

int spidev_get_opt(void *dev, int opt, const char *optarg, int src);

int spidev_set_para(void *dev, int mode, int maxclk,int bpw, char* devname, int src);

int spidev_open(void *dev, int use);

int spidev_send(void *dev, const char *tx, size_t len);

int spidev_transfer(void *dev, const char *tx, const char *rx, size_t len);

void spidev_close(void *dev);

char* spidev_get_devname(void *dev);


#endif
