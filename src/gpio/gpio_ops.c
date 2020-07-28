#include "gpio_ops.h"
#include <unistd.h>

const char rising_str[] = "rising";
const char falling_str[] = "falling";
const char both_str[] = "both";
const char none_str[] = "none";

static inline int gpio_file_exist(int gpio_no)
{
	char gpio_str[64];
	
	sprintf(gpio_str,SYSFS_GPIO_PATH,gpio_no);
	
	return access(gpio_str, F_OK);
}

int gpio_export(int gpio_no)
{
	int fd = -1;
	int ret = 0;
	char gpio_str[10];
	int gpio_str_len = 0;

	if(gpio_file_exist(gpio_no)){
		fd = open(SYSFS_GPIO_EXPORT,O_WRONLY);
		if(fd < 0){
			ret = fd;
			goto exit;
		}
		sprintf(gpio_str,"%d",gpio_no);
		gpio_str_len = strlen(gpio_str) + 1;
		
		ret = write(fd,gpio_str,gpio_str_len);
		if(gpio_str_len != ret){
			ret = -ENOSPC;
			goto exit;
		}
	}
exit:

	if(fd >= 0){
		close(fd);
	}
	return ret;
}

int gpio_unexport(int gpio_no)
{
	int fd = -1;
	int ret = 0;
	char gpio_str[10];
	int gpio_str_len = 0;

	if(gpio_file_exist(gpio_no)){
		fd = open(SYSFS_GPIO_UNEXPORT,O_WRONLY);
		if(fd < 0){
			ret = fd;
			goto exit;
		}
		sprintf(gpio_str,"%d",gpio_no);
		gpio_str_len = strlen(gpio_str) + 1;
		
		ret = write(fd,gpio_str,gpio_str_len);
		if(gpio_str_len != ret){
			ret = -ENOSPC;
			goto exit;
		}
	}
exit:

	if(fd >= 0){
		close(fd);
	}
	return ret;
}



int gpio_set_dir(int gpio_no, unsigned char dir)
{

	int ret = 0;
	char gpio_path[64];
	int gpio_str_len = 0;
	int fd;
	
	snprintf(gpio_path, ARRAY_SIZE(gpio_path), SYSFS_GPIO_DIR, gpio_no);

	fd = open(gpio_path,O_RDWR | O_SYNC, 0666);
	if(fd < 0){
		ret = fd;
		goto exit;
	}
	
	if(GPIO_DIR_OUT == dir){
		gpio_str_len = strlen(SYSFS_GPIO_DIR_OUT) + 1;
		ret = write(fd,SYSFS_GPIO_DIR_OUT,gpio_str_len);
	}else{
		gpio_str_len = strlen(SYSFS_GPIO_DIR_IN) + 1;
		ret = write(fd,SYSFS_GPIO_DIR_IN,gpio_str_len);
	}
	
	if(gpio_str_len != ret){
		ret = -ENOSPC;
		goto exit;
	}
exit:

	if(fd >= 0){
		close(fd);
	}

	return ret;	
}

int gpio_set_val(int gpio_no, unsigned char val)
{
	int ret;
	char gpio_path[64];
	int gpio_str_len = 0;
	int fd;
	
	sprintf(gpio_path,SYSFS_GPIO_VAL,gpio_no);
	
	fd = open(gpio_path,O_RDWR | O_SYNC, 0666);
	if(fd < 0){
		ret = fd;
		goto exit;
	}
	//lseek(fd,0,SEEK_SET);

//bugs : 再次设置为out时，对应IO有毛刺
	if(val){
		gpio_str_len = strlen(SYSFS_GPIO_H) + 1;
		ret = write(fd,SYSFS_GPIO_H,gpio_str_len);
	}else{
		gpio_str_len = strlen(SYSFS_GPIO_L) + 1;
		ret = write(fd,SYSFS_GPIO_L,gpio_str_len);
	}
	
	if(gpio_str_len != ret){
		ret = -ENOSPC;
		goto exit;
	}
exit:
	
	if(fd >= 0){
		close(fd);
	}
	
	return ret;		
}

int gpio_get_val(int gpio_no, unsigned char *val)
{
	int ret;
	char gpio_path[64];
	int gpio_str_len = 0;
	int fd;
	
	snprintf(gpio_path, ARRAY_SIZE(gpio_path), SYSFS_GPIO_VAL, gpio_no);

	fd = open(gpio_path,O_RDONLY, 0666);
	if(fd < 0){
		ret = fd;
		goto exit;
	}

	//lseek(fd,0,SEEK_SET);
	memset(gpio_path,0,3);
	ret = read(fd,gpio_path,3);
	if(ret < 0){
		goto exit;
	}
	
	if(val){
		*val = (unsigned char)atoi(gpio_path);
	}

exit:

	if(fd >= 0){
		close(fd);
	}

	return ret;		
}

int gpio_get_val_fd(int fd, unsigned char *val)
{
	int ret = -EINVAL;

	if(fd >= 0){
		lseek(fd,0,SEEK_SET);
		ret = read(fd,val,sizeof(*val));
		*val = *val - 48;
	}
	return ret;
}


int gpio_set_intr(int gpio_no, int edge, int *poll_fd)
{
	int ret;
	char gpio_path[64];
	int gpio_str_len = 0;
	const char *pstr = NULL;
	int pstr_len = 0;
	int fd;
	int need_clr = 0;
	int v;

	if(NULL == poll_fd){
		ret = -EINVAL;
		goto exit;
	}
	sprintf(gpio_path,SYSFS_GPIO_EDGE,gpio_no);
	
	fd = open(gpio_path,O_RDWR | O_SYNC, 0666);
	if(fd < 0){
		ret = fd;
		goto exit;
	}

	switch(edge){
		case RISING:
			//ret = write(fd, RISING_STR, strlen(RISING_STR) + 1);
			pstr = rising_str;
			break;

		case FALLING:
			pstr = falling_str;
			break;

		case BOTH:
			pstr = both_str;
			break;

		case NONE:
		default:
			pstr = none_str;
			need_clr = 1;
			break;
	}
	pstr_len = strlen(pstr) + 1;

	ret = write(fd, pstr, pstr_len);
	if(pstr_len != ret){
		ret = -ENOSPC;
		goto exit;
	}

	if(need_clr){
		if(*poll_fd){
			close(*poll_fd);
			*poll_fd = -1;
		}
	}else{
		sprintf(gpio_path,SYSFS_GPIO_VAL,gpio_no);
		
		*poll_fd = open(gpio_path,O_RDWR | O_SYNC, 0666);
		if(*poll_fd < 0){
			ret = *poll_fd;
			goto exit;
		}
		lseek(*poll_fd,0,SEEK_SET);
		read(*poll_fd, &v,sizeof(v));
	}
		
exit:

	if(fd >= 0){
		close(fd);
	}

	return ret;		
}

int gpio_op(int gpio_no, unsigned char dir, unsigned char *val)
{
	int ret;

	if(NULL == val){
		ret = -EINVAL;
		goto exit;
	}
	
	ret = gpio_export(gpio_no);
	if(ret < 0){
		goto exit;
	}
	
	ret = gpio_set_dir(gpio_no, dir);
	if(ret < 0){
		goto exit;
	}

	if(GPIO_DIR_OUT == dir){
		ret = gpio_set_val(gpio_no, *val);
		if(ret < 0){
			goto exit;
		}
	}else{
		ret = gpio_get_val(gpio_no, val);
		if(ret < 0){
			goto exit;
		}
	}


exit:
	return ret;
}
