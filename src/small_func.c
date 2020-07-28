#include "core.h"
#include <limits.h>

static unsigned long long ret_ERANGE(void)
{
	errno = ERANGE; /* this ain't as small as it looks (on glibc) */
	return 0;//ULLONG_MAX;
}

static int bb_ascii_isalnum(unsigned char a)
{
	unsigned char b = a - '0';
	if (b <= 9)
		return (b <= 9);
	b = (a|0x20) - 'a';
	return b <= 'z' - 'a';
}

static unsigned long long handle_errors(unsigned long long v, char **endp)
{
	char next_ch = **endp;

	/* errno is already set to ERANGE by strtoXXX if value overflowed */
	if (next_ch) {
		/* "1234abcg" or out-of-range? */
		if (bb_ascii_isalnum(next_ch) || errno)
			return ret_ERANGE();
		/* good number, just suspicious terminator */
		errno = EINVAL;
	}
	return v;
}

int bb_strtoull(const char *arg, char **endp, int base){
	unsigned long long v;
	char *endptr;
	int index = 0;
	if (!endp) endp = &endptr;
	*endp = (char*) arg;
	/* strtoul("  -4200000000") returns 94967296, errno 0 (!) */

	/* I don't think that this is right. Preventing this... */

	if (!bb_ascii_isalnum(arg[0])){
		if('-' == arg[0]){
			index = 1;
		}else{
			return ret_ERANGE();
		}
	}

	/* not 100% correct for lib func, but convenient for the caller */
	errno = 0;
	v = strtoull(&arg[index], endp, base);
	v = handle_errors(v, endp);
	return index?-v:v;
}

char* skip_whitespace(const char *s)
{
	/* In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	while (*s == ' ' || (unsigned char)(*s - 9) <= (13 - 9))
		s++;

	return (char *) s;
}

char* skip_spec_c(const char *s, char c, int *found)
{
	while (*s == c || *s == ' ' || (unsigned char)(*s - 9) <= (13 - 9)){
		s++;
		if(found && (*s == c)){
			*found++;
		}
	}

	return (char *) s;

}

int str_c_count(const char *s, char c)
{
	int cnt = 0;

	for( ;*s != '\0';s++){
		if(*s == c){
			cnt++;
		}
	}

	return cnt;
}


