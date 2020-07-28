#include "cf_ops.h"
#include "core.h"

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

parse_type* parse_config_file(const char *file_buf)
{
	return cJSON_Parse(file_buf); 
}

void parse_config_del(parse_type* type)
{
	cJSON_Delete(type);
}


int get_item_hex_val(const parse_type *object, const char *val_name, int *val)
{
	int ret = 0;
	parse_type * json_name;
	
	if(!(object && val_name && val)){
		ret = -EINVAL;
		goto exit;
	}
	
	json_name = cJSON_GetObjectItem( object , val_name);
	if (cJSON_IsString(json_name) && (json_name->valuestring != NULL)){
		ret = -EINVAL;
		goto exit;
	}
		
	*val = bb_strtoull(json_name->valuestring, NULL, 0);

exit:
	return ret;
}

int get_item_val(const parse_type *object, const char *val_name, int *val)
{
	int ret = 0;
	parse_type * json_name;
	char hex_str_lc[] = "0x";
	char hex_str_uc[] = "0X";
	int base = 0;
	
	if(!(object && val_name && val)){
		ret = -EINVAL;
		goto exit;
	}
	
	json_name = cJSON_GetObjectItem( object , val_name);
	
	if (cJSON_IsString(json_name) && (json_name->valuestring != NULL)){

		if(strstr(json_name->valuestring,hex_str_lc) || \
			strstr(json_name->valuestring,hex_str_uc)){
			base = 16;
		}
			
		*val = bb_strtoull(json_name->valuestring, NULL, base);
	}else{
		ret = -EINVAL;
		*val = 0;
		goto exit;
	}
	
exit:
	return ret;
}



int get_item_int_val(const parse_type *object, const char *val_name, int *val)
{
	int ret = 0;
	parse_type * json_name;
	
	if(!(object && val_name && val)){
		ret = -EINVAL;
		goto exit;
	}
	
	json_name = cJSON_GetObjectItem( object , val_name);
	if (!cJSON_IsNumber(json_name) || !cJSON_IsNumber(json_name)){
		ret = -EINVAL;
		goto exit;
	}
	
	*val = json_name->valueint;

exit:
	return ret;
}

int get_array_size(const parse_type *object, const char *obj_name)
{
	int ret;
	parse_type *json_list;

	if(!(object && obj_name)){
		ret = -EINVAL;
		goto exit;
	}
	
	json_list = cJSON_GetObjectItem( object , obj_name);
	if(NULL == json_list){
		ret = -EINVAL;
		goto exit;
	}
	ret = cJSON_GetArraySize(json_list); 

exit:
	return ret;
}

parse_type* get_obj_item(const parse_type *object, const char *obj_name)
{
	return cJSON_GetObjectItem(object, obj_name); 
}

int get_item_str_val(const parse_type *object, const char *val_name, char *val)
{
	int ret = 0;
	parse_type * json_name;
	
	if(!(object && val_name && val)){
		ret = -EINVAL;
		goto exit;
	}
	
	json_name = cJSON_GetObjectItem( object , val_name);
	if (cJSON_IsString(json_name) && (json_name->valuestring != NULL)){
		strcpy(val, json_name->valuestring);
	}else{
		ret = -EINVAL;
		goto exit;
	}
	
exit:
	return ret;
}


