#ifndef __GPIO_OPS_H__
#define	__GPIO_OPS_H__

#include "core.h"

#define	GPIO_DIR_OUT	1
#define	GPIO_DIR_IN 	0

#define	NONE		0
#define	RISING		1
#define	FALLING		2
#define BOTH		(RISING | FALLING)
#define INTR_UNDEF	66


#define	SYSFS_GPIO_EXPORT	"/sys/class/gpio/export"
#define	SYSFS_GPIO_UNEXPORT	"/sys/class/gpio/unexport"

#define	SYSFS_GPIO_DIR		"/sys/class/gpio/gpio%d/direction"
#define	SYSFS_GPIO_VAL		"/sys/class/gpio/gpio%d/value"
#define	SYSFS_GPIO_EDGE		"/sys/class/gpio/gpio%d/edge"


#define	SYSFS_GPIO_PATH		"/sys/class/gpio/gpio%d"


#define	SYSFS_GPIO_H	"1"
#define	SYSFS_GPIO_L 	"0"

#define	SYSFS_GPIO_DIR_OUT	"out"
#define	SYSFS_GPIO_DIR_IN 	"in"

#define	RISING_STR		"rising"
#define	FALLING_STR		"falling"
#define BOTH_STR		"both"
#define	NONE_STR		"none"



int gpio_unexport(int gpio_no);
int gpio_export(int gpio_no);
int gpio_get_val(int gpio_no, unsigned char *val);
int gpio_get_val_fd(int fd, unsigned char *val);
int gpio_set_val(int gpio_no, unsigned char val);
int gpio_set_dir(int gpio_no, unsigned char dir);
int gpio_set_intr(int gpio_no, int edge, int *poll_fd);
int gpio_op(int gpio_no, unsigned char dir, unsigned char *val);

#endif
