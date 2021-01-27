
#include "tool.h"
#include "str2digit.h"
#include "spi_gpio_ops.h"
#include "cmd_line.h"

void write_to_5682_in_mode_1(int instuction){
	spi_write((char*)(&instuction), 3);

	cs_disable();
}

int main(int argc, char *argv[])
{
	int ret;
	int instuction;
	if(argc == 2){ //5682_spi_tool [instuction]
		instuction = bb_strtoull(argv[1], NULL, 16);
		instuction = htonl(instuction) >> 8;
		printf("instuction : %s , 0x%x \n", argv[1], instuction);
    }else{
		printf("usage : 5682_spi_tool [instuction]\n");
		return 0;
	}

	if(simulate_spi_init() < 0){
		printf("spi gpio init failed ! \n");
		return 0;
	}

	write_to_5682_in_mode_1(instuction);

	printf(" complete spi write \n");

	return 0;
}

