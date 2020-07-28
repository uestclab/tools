#ifndef	__I2C_COMMON_H__
#define	__I2C_COMMON_H__
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
#include <linux/i2c.h>
#include <linux/i2c-dev.h>


#define	CF_DEV	0
#define	CT_DEV	1
#define	CL_DEV	2
#define	TM_DEV	3

#define	ZERO_MODE	0
#define	BYTE_MODE	1
#define	WORD_MODE	2
#define	BLOCK_MODE	3

#define	DEV_NAME_LEN	256

#define	MAX_DEV		2

struct i2cdev_para_s{
	char i2cdev[DEV_NAME_LEN];
	int size;
	int addr;
	int force;
	
	void *priv;
};

struct i2c_dev_s{
	int in_used;
	
	int fd;
	unsigned long funcs;
	
	struct	i2cdev_para_s	*dev_use; // poit to cl_dev cf_dev ct_dev
	
	struct	i2cdev_para_s	cl_dev;
	struct	i2cdev_para_s	cf_dev;
	struct	i2cdev_para_s	ct_dev;
	struct	i2cdev_para_s	tm_dev;
};


/*#define	LONG_I2C_DEV_OPT\
	{ "i2cdev",  1, NULL, 'i' },\
	{ "mode",    1, NULL, 'm' },\
	{ "clk",     1, NULL, 'c' },\
	{ "bpw",     1, NULL, 'b' }
*/
//#define	SHORT_SPI_DEV_OPT	"i:m:c:b:"

/*
#define	SPI_DEV_HELP_INFO\
	" -i --spidev\t\tspi device path(eg. /dev/spidev1.0 )\n"\
	" -m --mode\t\tspi mode(0\1\2\3)\n"\
	" -c --clk\t\tspi max clk(Hz)\n"\
	" -b --bpw\t\tspi bit perword\n"*/

#define	set_size(i2cdev, len) \
do{\
	switch(len){\
	case BYTE_MODE: //BYTE \
		i2cdev->dev_use.size = I2C_SMBUS_BYTE_DATA; \
		break; \
	case WORD_MODE://WORD \
		i2cdev->dev_use.size = I2C_SMBUS_BLOCK_DATA; \
		break; \
	default://BLOCK \
		i2cdev->dev_use.size = I2C_SMBUS_I2C_BLOCK_DATA; \
		break; \
	} \
}while(1)

void i2cdev_init(void **dev);
void i2cdev_set_user_conf(void* dev, int use);
int i2cdev_set_para(void *dev, int size, int addr, int force, char* devname, int src);
int set_slave_addr(int file, int address, int force);
int i2cdev_open(void *dev, int use);
void i2cdev_close(void *dev);
char* i2cdev_get_devname(void *dev);

int i2c_read_byte(void *dev, char *val);
int i2c_write_byte(void *dev, unsigned char val);
int i2c_read_byte_data(void *dev, unsigned char *val, unsigned char cmd);
int i2c_write_byte_data(void *dev, unsigned char val, unsigned char cmd);
int i2c_read_word_data(void *dev, unsigned short *val, unsigned char cmd);
int i2c_write_word_data(void *dev, unsigned short val, unsigned char cmd);
int i2c_read_block_data(void *dev, unsigned char *val, int val_len, unsigned char cmd);

static inline int i2c_smbus_access(int file, char read_write, unsigned char command,
		       int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;
	int err;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;

	err = ioctl(file, I2C_SMBUS, &args);
	if (err == -1)
		err = -errno;
	return err;
}

#endif
