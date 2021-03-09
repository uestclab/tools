#ifndef _SPI_GPIO_OPS_H
#define _SPI_GPIO_OPS_H

#include "gpio_ops.h"

/*使用什么模式就将对应宏定义改成1*/
#define 				SPI_Mode_0               0
#define 				SPI_Mode_1				 1
#define 				SPI_Mode_2				 0
#define 				SPI_Mode_3				 0


#define SPI_SDI_AD5682    1020
#define SPI_SYNC_AD5682   1019
#define SPI_LDAC_AD5682   1021
#define SPI_SCLK_AD5682   1018


int simulate_spi_init(int spi_delay_time_us, int mode);
void spi_write (unsigned char* buf, int len);

#endif

