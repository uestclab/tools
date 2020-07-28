#ifndef	__GPIO_DEV_H__

#define	__GPIO_DEV_H__

typedef  int (*intr_func) (void* dev, void *arg);

#define	gpio_dev_del(dev) \
do{\
	if(dev){\
		free(dev);\
		dev = NULL; \
	}\
}while(0)


#define	MAX_POLL	16

struct gpio_poll_s{
	int cnt;
	struct pollfd fds[MAX_POLL];
	void *dev_fds[MAX_POLL];
};

struct gpio_dev_s{
	int fd;
	int no;
	int edge;
	int dir;
	unsigned char val;

	
	intr_func gpio_intr_fn;
	void* fn_arg;

	int index;
	void *priv;
};

int gpio_layer_init(void);
int gpio_dev_create(void** dev);
int gpio_open(void* dev, int gpio_no, unsigned char dir);
int gpio_set(void* dev, unsigned char val);
int gpio_get(void* dev, unsigned char *val);
int gpio_register_intr(void *dev, intr_func intr_fn, int edge, void *fn_arg);
int gpio_unregister_intr(void *dev);
int gpio_poll_waite(int to_ms);
int gpio_close(void* dev);
int gpio_poll_cnt(void);


static inline int get_dev_no(void* dev)
{
	struct gpio_dev_s *pdev = (struct gpio_dev_s*)dev;

	return pdev->no;
}



#endif
