#ifndef	__CONF_OPS_FILE_H__
#define	__CONF_OPS_FILE_H__

#include"cJSON.h" 

typedef	cJSON	parse_type;
int read_config_file(const char* path, char** file_buf);
parse_type* parse_config_file(const char *file_buf);
void parse_config_del(parse_type* type);
int get_item_hex_val(const parse_type *object, const char *val_name, int *val);
int get_item_val(const parse_type *object, const char *val_name, int *val);
int get_item_int_val(const parse_type *object, const char *val_name, int *val);
int get_array_size(const parse_type *object, const char *obj_name);
parse_type* get_obj_item(const parse_type *object, const char *obj_name);
int get_array_val(int index, const char *val_name, parse_type *js_list, int *val);
int get_item_str_val(const parse_type *object, const char *val_name, char *val);

#define get_ArrayForEach(element, array) cJSON_ArrayForEach(element, array)
#endif
