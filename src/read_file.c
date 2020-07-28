
#include "tool.h"

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


char* get_json_buf(char* path){
    char *file_buf;

    int ret = read_config_file(path, &file_buf);

	if(ret > 0){
		return file_buf;
	}else{
		return NULL;
	}
}