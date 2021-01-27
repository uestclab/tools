#include "spi_gpio_ops.h"
#include <stdio.h>
#include <unistd.h>

/*clk*/
#define set_clk_pin_output()
/*miso*/
#define set_miso_pin_input() 
/*mosi*/
#define set_mosi_pin_output()
/*cs*/
#define set_cs_pin_output()

#define write_gpio_high()  gpio_set_val(SPI_SDI_AD5682, 1) 
#define write_gpio_low()   gpio_set_val(SPI_SDI_AD5682, 0) 
#define write_clk_high()   gpio_set_val(SPI_SCLK_AD5682, 1) 
#define write_clk_low()    gpio_set_val(SPI_SCLK_AD5682, 0) 

#define cs_high()          gpio_set_val(SPI_SYNC_AD5682, 1)
#define cs_low()           gpio_set_val(SPI_SYNC_AD5682, 0)

#define read_gpio()

#define udelay(x)   usleep(x)

#define _SPI_DELAY_TIME_US_ 1000

/* init */
int simulate_spi_init(void)
{
    /*mosi*/
    if(gpio_open(SPI_SDI_AD5682, 1) < 0){
        return -1;
    }
    /*cs*/
    if(gpio_open(SPI_SYNC_AD5682, 1) < 0){
        return -1;
    }

    cs_high();

    if(gpio_open(SPI_LDAC_AD5682, 1) < 0){
        return -1;
    }

    if( gpio_set_val(SPI_LDAC_AD5682, 0) <0 ){
        return -1;
    }
    
    /*clk*/
    if(gpio_open(SPI_SCLK_AD5682, 1) < 0){
        return -1;
    }

    write_clk_low();
    udelay(_SPI_DELAY_TIME_US_);
    cs_low();

    return 0;
}

void cs_disable(){
    cs_high();
}

/* Mode_1 */
void simulate_spi_write_byte(unsigned char tx_data)
{
	int i = 0;
	for(i = 7;i >= 0;i--)
	{
        write_clk_high();		
		if(tx_data & (1 << i))
		{
			write_gpio_high();
            printf("1");
		}
		else
		{
			write_gpio_low();
            printf("0");
		}
        udelay(_SPI_DELAY_TIME_US_);
		write_clk_low();
		udelay(_SPI_DELAY_TIME_US_);		
	}
}

void spi_write (unsigned char* buf, int len)  
{  
    int i;
    for (i=0; i<len; i++){ 
        printf("\n buf[%d] = 0x%x \n", i, buf[i]);
        simulate_spi_write_byte(buf[i]);
    }  
}
