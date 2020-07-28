#include "spi_common.h"

#include "core.h"


struct spi_dev_s g_spidev;


void spidev_init(void **dev)
{
	*dev = (void*)&g_spidev;
	memset((void*)&g_spidev, 0, sizeof(struct spi_dev_s));
	g_spidev.fd = -1;
}

void spidev_set_user_conf(void* dev, int use)
{
	struct spi_dev_s *spidev = (struct spi_dev_s *)dev;
	
	switch(use){
		case CF_DEV:
			spidev->dev_use = &(spidev->cf_dev);
			break;

		case CT_DEV:
			spidev->dev_use = &(spidev->ct_dev);
			break;

		case CL_DEV:
		default:
			spidev->dev_use = &(spidev->cl_dev);
			break;
	}
}

int spidev_get_opt(void *dev, int opt, const char *optarg, int src)
{
	int ret = -EINVAL;
	struct spi_dev_s *spidev = (struct spi_dev_s *)dev;
	struct	spidev_para_s *para = NULL;

	if(NULL == spidev){
		ret = -ENODEV;
	}
	
	switch(src){
		case CF_DEV:
			para = &(spidev->cf_dev);
			break;

		case CT_DEV:
			para = &(spidev->ct_dev);
			break;

		case CL_DEV:
		default:
			para = &(spidev->cl_dev);
			break;
	}

	switch (opt) {
		case 'm':
			para->spimode = bb_strtoull(optarg, NULL, 0);
			ret = 0;
		    break;

		case 'c':
			para->spimaxclk = bb_strtoull(optarg, NULL, 0);
			ret = 0;
		    break;

		case 'b':
			para->spibpw = bb_strtoull(optarg, NULL, 0);
			ret = 0;
		    break;
			
		case 'i':
			strcpy(para->spidev, optarg);
			ret = 0;
			break;
			
		default: ;
	}

	return ret;
}

int spidev_set_para(void *dev, int mode, int maxclk,int bpw, char* devname, int src)
{
	int ret = 0;
	struct spi_dev_s *spidev = (struct spi_dev_s *)dev;
	struct	spidev_para_s *para = NULL;

	if(NULL == spidev){
		ret = -ENODEV;
	}
	
	switch(src){
		case CF_DEV:
			para = &(spidev->cf_dev);
			break;

		case CT_DEV:
			para = &(spidev->ct_dev);
			break;

		case CL_DEV:
		default:
			para = &(spidev->cl_dev);
			break;
	}

	para->spimode = mode;
	para->spimaxclk = maxclk;
	para->spibpw = 	bpw;	
	strcpy(para->spidev, devname);

	return ret;
}

static void spidev_init_ioc(void *dev)
{
	struct spi_dev_s *spidev = (struct spi_dev_s *)dev;

	spidev->tr.tx_buf = (unsigned long)NULL;
	spidev->tr.rx_buf = (unsigned long)NULL;
	spidev->tr.len = 0;
	spidev->tr.delay_usecs = 0;
	spidev->tr.speed_hz = spidev->dev_use->spimaxclk;
	spidev->tr.bits_per_word = spidev->dev_use->spibpw;
}


int spidev_open(void *dev, int use)
{
	int ret = 0;
	struct spi_dev_s *spidev = NULL;

	spidev = (struct spi_dev_s *)dev;
		
	if(NULL == spidev){
		ret = -EINVAL;
		goto exit;
	}

	if(spidev->fd >= 0){
		spidev_close(dev); 
	}

	spidev_set_user_conf(dev, use);

	spidev->fd = open(spidev->dev_use->spidev, O_RDWR);
	if (spidev->fd < 0){
		ret = spidev->fd;
		//log_error("can't open device");
		goto exit;
	}

	ret = ioctl(spidev->fd, SPI_IOC_WR_MODE32, &(spidev->dev_use->spimode));
	if (ret < 0){
		//log_error("can't set spi mode");
		goto exit;
	}

	ret = ioctl(spidev->fd, SPI_IOC_RD_MODE32, &(spidev->dev_use->spimode));
	if (ret < 0){
		//log_error("can't get spi mode");
		goto exit;
	}

	ret = ioctl(spidev->fd, SPI_IOC_WR_BITS_PER_WORD, &(spidev->dev_use->spibpw));
	if (ret < 0){
		//log_error("can't set bits per word");
		goto exit;
	}

	ret = ioctl(spidev->fd, SPI_IOC_RD_BITS_PER_WORD, &(spidev->dev_use->spibpw));
	if (ret < 0){
		//log_error("can't get bits per word");
		goto exit;
	}

	ret = ioctl(spidev->fd, SPI_IOC_WR_MAX_SPEED_HZ, &(spidev->dev_use->spimaxclk));
	if (ret < 0){
		//log_error("can't set max speed hz");
		goto exit;
	}

	ret = ioctl(spidev->fd, SPI_IOC_RD_MAX_SPEED_HZ, &(spidev->dev_use->spimaxclk));
	if (ret < 0){
		//log_error("can't get max speed hz");
		goto exit;
	}

	spidev_init_ioc(dev);
exit:
	return ret;
}


int spidev_send(void *dev, const char *tx, size_t len)
{
	int ret;
	int out_fd;
	struct spi_dev_s *spidev = (struct spi_dev_s *)dev;

	spidev->tr.tx_buf = (unsigned long)tx;
	spidev->tr.rx_buf = (unsigned long)NULL;
	spidev->tr.len = len;

	ret = ioctl(spidev->fd, SPI_IOC_MESSAGE(1), &(spidev->tr));
	
	/*if (ret < 1){
		log_error("can't send spi message(fd=0x%x, ret = 0x%x)",spidev.fd,ret);
	}*/

	return ret;
}


int spidev_transfer(void *dev, const char *tx, const char *rx, size_t len)
{
	int ret;
	int out_fd;
	struct spi_dev_s *spidev = (struct spi_dev_s *)dev;
	
	spidev->tr.tx_buf = (unsigned long)tx;
	spidev->tr.rx_buf = (unsigned long)rx;
	spidev->tr.len = len;

	ret = ioctl(spidev->fd, SPI_IOC_MESSAGE(1), &(spidev->tr));

	return ret;
}

void spidev_close(void *dev)
{
	struct spi_dev_s *spidev = (struct spi_dev_s *)dev;

	if(spidev->fd >= 0){
		close(spidev->fd);
		spidev->fd = -1;
	}
}

char* spidev_get_devname(void *dev)
{
	struct spi_dev_s *spidev = (struct spi_dev_s *)dev;

	return spidev->dev_use->spidev;
}
