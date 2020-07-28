#include "i2c_common.h"


struct i2c_dev_s g_i2cdev[MAX_DEV] = {
	{
		.in_used = 0,
	},
	{
		.in_used = 0,
	}
};

void i2cdev_init(void **dev)
{
	int i;
	
	for(i=0; i<MAX_DEV; i++){
		if(0 == g_i2cdev[i].in_used){
			break;
		}
	}

	if(MAX_DEV != i){
		*dev = (void*)&g_i2cdev[i];
		memset((void*)&g_i2cdev[i], 0, sizeof(struct i2c_dev_s));
		g_i2cdev[i].fd = -1;
		g_i2cdev[i].in_used = 1;
	}else{
		*dev = NULL;
	}
}

void i2cdev_set_user_conf(void* dev, int use)
{
	struct i2c_dev_s *i2cdev = (struct i2c_dev_s *)dev;
	
	switch(use){
		case CF_DEV:
			i2cdev->dev_use = &(i2cdev->cf_dev);
			break;

		case CT_DEV:
			i2cdev->dev_use = &(i2cdev->ct_dev);
			break;
		
		case TM_DEV:
			i2cdev->dev_use = &(i2cdev->tm_dev);
			break;

		case CL_DEV:
		default:
			i2cdev->dev_use = &(i2cdev->cl_dev);
			break;
	}
}

int i2cdev_set_para(void *dev, int size, int addr, int force, char* devname, int src)
{
	int ret = 0;
	struct i2c_dev_s *i2cdev = (struct i2c_dev_s *)dev;
	struct	i2cdev_para_s *para = NULL;

	if(NULL == i2cdev){
		ret = -ENODEV;
		goto exit;
	}
	
	switch(src){
		case CF_DEV:
			para = &(i2cdev->cf_dev);
			break;

		case CT_DEV:
			para = &(i2cdev->ct_dev);
			break;

		case TM_DEV:
			para = &(i2cdev->tm_dev);
			break;

		case CL_DEV:
		default:
			para = &(i2cdev->cl_dev);
			break;
	}

	//strcpy(para->i2cdev, devname);
	para->size = size;
	para->addr = addr;
	para->force = force;
	if(devname){
		snprintf(para->i2cdev, DEV_NAME_LEN, devname);
	}

exit:
	return ret;
}

static int get_funcs(int file, unsigned long *funcs)
{
	int ret = -EINVAL;

	if(funcs){
		/* check adapter functionality */
		ret = ioctl(file, I2C_FUNCS, funcs);
	}

#if 0
	switch (size) {
	/*case I2C_SMBUS_BYTE:
		if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE)) {
			//fprintf(stderr, MISSING_FUNC_FMT, "SMBus send byte");
			return -EINVAL;
		}
		break;*/

	case I2C_SMBUS_BYTE_DATA:
		if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE_DATA)) {
			//fprintf(stderr, MISSING_FUNC_FMT, "SMBus write byte");
			return -EINVAL;
		}
		break;

	case I2C_SMBUS_WORD_DATA:
		if (!(funcs & I2C_FUNC_SMBUS_WRITE_WORD_DATA)) {
			//fprintf(stderr, MISSING_FUNC_FMT, "SMBus write word");
			return -EINVAL;
		}
		break;

	case I2C_SMBUS_BLOCK_DATA:
		if (!(funcs & I2C_FUNC_SMBUS_WRITE_BLOCK_DATA)) {
			//fprintf(stderr, MISSING_FUNC_FMT, "SMBus block write");
			return -EINVAL;
		}
		break;
	case I2C_SMBUS_I2C_BLOCK_DATA:
		if (!(funcs & I2C_FUNC_SMBUS_WRITE_I2C_BLOCK)) {
			//fprintf(stderr, MISSING_FUNC_FMT, "I2C block write");
			return -EINVAL;
		}
		break;
	default :
		return -EINVAL;
	}
#endif
	return 0;
}

int set_slave_addr(int file, int address, int force)
{
	/* With force, let the user read from/write to the registers
	   even when a driver is also running */
	if (ioctl(file, force ? I2C_SLAVE_FORCE : I2C_SLAVE, address) < 0) {
		//fprintf(stderr,
		//	"Error: Could not set address to 0x%02x: %s\n",
		//	address, strerror(errno));
		return -errno;
	}

	return 0;
}


int i2cdev_open(void *dev, int use)
{
	int ret = 0;
	struct i2c_dev_s *i2cdev = NULL;
	unsigned long funcs;

	i2cdev = (struct i2c_dev_s *)dev;

	if(NULL == i2cdev){
		ret = -EINVAL;
		goto exit;
	}

	if(i2cdev->fd >= 0){
		i2cdev_close(dev); 
	}

	i2cdev_set_user_conf(dev, use);
	i2cdev->fd = open(i2cdev->dev_use->i2cdev, O_RDWR);
	if(i2cdev->fd < 0){
		ret = -errno;
		goto exit;
	}

	//ret = ioctl(i2cdev->fd, I2C_FUNCS, &funcs);

	//printf("%s-%d\n",__func__, funcs);
	/*ret = check_funcs(i2cdev->fd, i2cdev->dev_use->size);
	if(ret){
		goto exit;
	}*/
	
	ret = set_slave_addr(i2cdev->fd, i2cdev->dev_use->addr, i2cdev->dev_use->force);
exit:
	return ret;
}


void i2cdev_close(void *dev)
{
	struct i2c_dev_s *i2cdev = (struct i2c_dev_s *)dev;

	if(dev && (i2cdev->fd >=0)){
		close(i2cdev->fd);
		i2cdev->fd = -1;
	}
}

char* i2cdev_get_devname(void *dev)
{
	struct i2c_dev_s *i2cdev = (struct i2c_dev_s *)dev;

	return i2cdev->dev_use->i2cdev;
}

int i2c_read_byte(void *dev, char *val)
{
	struct i2c_dev_s *i2cdev = (struct i2c_dev_s *)dev;
	union i2c_smbus_data data;	
	int err;

	if(!dev || !val){
		return -EINVAL;
	}
	err = i2c_smbus_access(i2cdev->fd, I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data);	
	if (err < 0)		
		return err;	
	
	*val = data.byte;
	
	return 0;
}

int i2c_read_byte_data(void *dev, unsigned char *val, unsigned char cmd)
{
	struct i2c_dev_s *i2cdev = (struct i2c_dev_s *)dev;
	union i2c_smbus_data data;	
	int err;

	if(!(dev || val)){
		return -EINVAL;
	}
	
	err = i2c_smbus_access(i2cdev->fd, I2C_SMBUS_READ, cmd, 
				I2C_SMBUS_BYTE_DATA, &data);	
	if (err < 0)		
		return err;	
	
	*val = data.byte;
	
	return 0;
}

int i2c_write_byte(void *dev, unsigned char cmd)
{
	struct i2c_dev_s *i2cdev = (struct i2c_dev_s *)dev;
	int err;

	if(!dev){
		return -EINVAL;
	}
	
	return i2c_smbus_access(i2cdev->fd, I2C_SMBUS_WRITE, cmd,
				I2C_SMBUS_BYTE, NULL);
}

int i2c_write_byte_data(void *dev, unsigned char val, unsigned char cmd)
{
	struct i2c_dev_s *i2cdev = (struct i2c_dev_s *)dev;
	union i2c_smbus_data data;	
	int err;

	if(!dev){
		return -EINVAL;
	}
	
	data.byte = val;
	return i2c_smbus_access(i2cdev->fd, I2C_SMBUS_WRITE, cmd,
				I2C_SMBUS_BYTE_DATA, &data);
}

int i2c_read_word_data(void *dev, unsigned short *val, unsigned char cmd)
{
	struct i2c_dev_s *i2cdev = (struct i2c_dev_s *)dev;
	union i2c_smbus_data data;	
	int err;

	if(!(dev || val)){
		return -EINVAL;
	}
	err = i2c_smbus_access(i2cdev->fd, I2C_SMBUS_READ, cmd, 
				I2C_SMBUS_WORD_DATA, &data);	
	if (err < 0)		
		return err;	
	
	*val = data.word;
	
	return 0;
}


int i2c_write_word_data(void *dev, unsigned short val, unsigned char cmd)
{
	struct i2c_dev_s *i2cdev = (struct i2c_dev_s *)dev;
	union i2c_smbus_data data;	
	int err;

	if(!dev){
		return -EINVAL;
	}
	
	data.word = val;
	return i2c_smbus_access(i2cdev->fd, I2C_SMBUS_WRITE, cmd,
				I2C_SMBUS_BYTE_DATA, &data);
}

int i2c_read_block_data(void *dev, unsigned char *val, int val_len, unsigned char cmd)
{
	struct i2c_dev_s *i2cdev = (struct i2c_dev_s *)dev;
	union i2c_smbus_data data;	
	int err;
	int i;
	int loop;

	if(!(dev || val)){
		return -EINVAL;
	}
	
	err = i2c_smbus_access(i2cdev->fd, I2C_SMBUS_READ, cmd, 
				I2C_SMBUS_BLOCK_DATA, &data);	
	if (err < 0)		
		return err;	

	loop = (data.block[0] > val_len)?val_len:data.block[0];
	
	for (i = 1; i <= loop; i++)		
		val[i-1] = data.block[i];
	
	return data.block[0];
}



