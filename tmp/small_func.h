#ifndef	__SMALL_FUNC_H__
#define	__SMALL_FUNC_H__


int bb_strtoull(const char *arg, char **endp, int base);

char* skip_spec_c(const char *s, char c, int *found);
char* skip_whitespace(const char *s);
int str_c_count(const char *s, char c);
#endif



