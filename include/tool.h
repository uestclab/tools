#ifndef _MY_TOOL_H
#define _MY_TOOL_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <poll.h>
#include "list.h"


#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static inline void* xmalloc(size_t size)
{
	void *ptr = malloc(size);
	return ptr;
}

static inline void* xzalloc(size_t size)
{
	void *ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

static inline void* xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	return ptr;
}

#define	xfree(ptr) do{ \
	if(ptr){ \
		free(ptr); \
		ptr = NULL; \
	} \
}while(0)

static inline void get_usec(struct timeval *time)
{
	gettimeofday(time, NULL);

}

static inline int time_expire(struct timeval start, int expire_us)
{
	struct timeval now;
	int sec;
	int diff_usec;
	
	gettimeofday(&now, NULL);

	sec = now.tv_sec - start.tv_sec;
	
	diff_usec = sec * 1000000 + now.tv_usec - start.tv_usec;

	return ((diff_usec - expire_us)>=0)?1:0;
}


#endif