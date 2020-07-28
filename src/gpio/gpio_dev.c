#include<common.h>
#include<gpio/gpio_dev.h>
#include<gpio/gpio_ops.h>

struct gpio_poll_s g_poll_list = {
	.cnt = 0,
};

//修改建议:  链表
//If this field is negative, then the corresponding events field is ignored and the revents field returns zero.
static inline int fds_del(struct gpio_poll_s *pfds, void* dev, int index)
{

	int i = 0;
	int ret = -EINVAL;

	if(pfds->cnt > 0){
		if((index < MAX_POLL) && (index >=0 )){
			pfds->fds[index].fd = -1;
			pfds->fds[index].revents = 0;
			pfds->cnt--;
			ret = 0;
		}
	}

	return ret;
}

//先采用线性搜索
static inline int fds_add(struct gpio_poll_s *pfds)
{

	int i = 0;
	int found = -EMFILE;

	for(i=0; i< MAX_POLL; i++){
		if(-1 == pfds->fds[i].fd){
			found = i;
			break;
		}
	}

	return found;
}


int gpio_layer_init(void)
{
	int ret = 0;
	int i = 0;
	
	g_poll_list.cnt = 0;

	for(i=0; i< MAX_POLL; i++){
		g_poll_list.fds[i].fd = -1;
		g_poll_list.dev_fds[i] = NULL;
	}
}


int gpio_dev_create(void** dev)
{
	int ret = 0;
	
	struct gpio_dev_s *pdev = NULL;

	if(*dev){
		//设备存在，  重复添加的情况 会发生
		goto exit;
	}
	pdev = (struct gpio_dev_s*)xzalloc(sizeof(struct gpio_dev_s));

	if(NULL == pdev){
		ret = -ENOMEM;
		goto exit;
	}

	pdev->dir = GPIO_DIR_IN;
	pdev->fd = -1;
	pdev->fn_arg = NULL;
	pdev->gpio_intr_fn = NULL;
	pdev->priv = NULL;
	
	*dev = (void*)pdev;

exit:		
	return ret;
}



int gpio_open(void* dev, int gpio_no, unsigned char dir)
{
	int ret = 0;
	int fd = -1;
	
	struct gpio_dev_s *pdev = (struct gpio_dev_s*)dev;

	ret = gpio_export(gpio_no);
	if(ret < 0){
		goto exit;
	}

	ret = gpio_set_dir(gpio_no, dir);
	if(ret < 0){
		goto exit;
	}


	pdev->no = gpio_no;
	pdev->fd = fd;
	pdev->dir = dir;
	pdev->priv = &g_poll_list;
	
exit:
	return (ret > 0)?0:ret;
}


int gpio_set(void* dev, unsigned char val)
{
	int ret = 0;
	int fd = 0;

	struct gpio_dev_s *pdev = (struct gpio_dev_s*)dev;

	ret = gpio_set_val(pdev->no, val);
	if(ret < 0){
		goto exit;
	}

	pdev->val = val;
exit:
	return (ret > 0)?0:ret;
}


int gpio_get(void* dev, unsigned char *val)
{
	int ret = 0;
	int fd = 0;

	struct gpio_dev_s *pdev = (struct gpio_dev_s*)dev;

	if(NULL == val){
		ret = -EINVAL;
		goto exit;
	}

	if(pdev->fd >= 0){
		ret = gpio_get_val_fd(pdev->fd,val);
	}else{
		ret = gpio_get_val(pdev->no, val);
	}
	if(ret < 0){
		goto exit;
	}

	pdev->val = *val;
	
exit:
	return (ret > 0)?0:ret;
}


int gpio_register_intr(void *dev, intr_func intr_fn, int edge, void *fn_arg)
{
	int ret = 0;
	int fd = 0;
	struct gpio_dev_s *pdev = (struct gpio_dev_s*)dev;
	struct gpio_poll_s *pfds = (struct gpio_poll_s *)pdev->priv;

	ret = gpio_set_intr(pdev->no, edge, &(pdev->fd));
	if(ret < 0){
		goto exit;
	}
	
	ret = fds_add(pfds);
	if(ret < 0){
		goto exit;
	}
	
	pdev->edge = edge;
	pdev->gpio_intr_fn = intr_fn;
	pdev->fn_arg = fn_arg;
	pdev->index = ret;
	
	pfds->fds[ret].fd  = pdev->fd;
	pfds->fds[ret].events  = POLLPRI;
	pfds->fds[ret].revents = 0;
	pfds->dev_fds[ret] = dev;
	
	pfds->cnt++;

exit:	
	return (ret > 0)?0:ret;
}


int gpio_unregister_intr(void *dev)
{
	int ret = 0;
	int fd = 0;
	struct gpio_dev_s *pdev = (struct gpio_dev_s*)dev;
	struct gpio_poll_s *pfds = (struct gpio_poll_s *)pdev->priv;

	ret = gpio_set_intr(pdev->no, NONE, &(pdev->fd));
	if(ret < 0){
		goto exit;
	}
	
	pdev->edge = NONE;
	pdev->gpio_intr_fn = NULL;

	ret = fds_del(pfds, dev, pdev->index);
	pfds->dev_fds[pdev->index] = NULL;
	pdev->index = -1;

exit:
	return (ret > 0)?0:ret;
}

int gpio_poll_waite(int to_ms)
{
	int ret = 0, rc; 
	int i, found = 0;
	int v = 0;

	struct gpio_dev_s *pdev = NULL;
	
	rc = poll(g_poll_list.fds,MAX_POLL,to_ms);
	if(rc > 0 ){
		for(i=0; i<MAX_POLL; i++ ){
			if(g_poll_list.fds[i].revents & POLLPRI){
				g_poll_list.fds[i].revents = 0;
				lseek(g_poll_list.fds[i].fd,0,SEEK_SET);
				read(g_poll_list.fds[i].fd, &v,sizeof(v));
				found++;
				pdev = (struct gpio_dev_s *)g_poll_list.dev_fds[i];
				if(pdev->gpio_intr_fn){
					ret |= pdev->gpio_intr_fn((void*)pdev,pdev->fn_arg);
				}
				if(found == rc){
					break;
				}
			}
		}
	}

	
	return (ret > 0)?0:ret;
}

int gpio_close(void* dev)
{
	int ret = 0;
	struct gpio_dev_s *pdev = (struct gpio_dev_s*)dev;

	if(dev){
	//unexport ????
	//ret = gpio_unexport(pdev->no);  //保持IO状态
		pdev->no = -1;

		
		pdev->fd = -1;
		pdev->dir = 0;

		pdev->priv = NULL;
	}

	return ret;
}

int gpio_poll_cnt(void)
{
	int ret = 0;
	struct gpio_poll_s *pfds = (struct gpio_poll_s *)&g_poll_list;

exit:
	return pfds->cnt;
}



