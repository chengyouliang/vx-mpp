#include "mpp_dmabuf.h"

struct mpp_dmabuf_attachment {
	struct sg_table sgt;
	enum dma_data_direction dma_dir;
};
struct mpp_dmabuf_priv {
	struct mpp_dma_buffer *buffer;
	/* DMABUF related */
	struct device *dev;
	struct sg_table *sgt_base;
	enum dma_data_direction dma_dir;
	struct list_head   list;
	struct mpp_dmabuf_attachment *attachment;
	int *fd;
};

typedef struct _mpp_dmabuf_info
{
	/* data */
	struct list_head      dma_list;
	struct device         *dma_device;
	struct mutex          dma_lock;
} mpp_dmabuf_info;


static mpp_dmabuf_info *gdma_buf_info;


void mpp_dmabuf_hander_release(struct mpp_dmabuf_priv* mpp_hander)
{
	struct mpp_dma_buffer *buffer = mpp_hander->buffer;
		
	struct sg_table *sgt;
	
	if (mpp_hander->attachment)
	{
		sgt = &mpp_hander->attachment->sgt;
		/* release the scatterlist cache */
		if (mpp_hander->attachment->dma_dir != DMA_NONE)
			dma_unmap_sg(mpp_hander->dev, sgt->sgl, sgt->orig_nents,
			     	mpp_hander->attachment->dma_dir);
		kfree(mpp_hander->attachment);
	}
	
	if (mpp_hander->sgt_base) {
		sg_free_table(mpp_hander->sgt_base);
		kfree(mpp_hander->sgt_base);
	}

	if (buffer)
	{
		dma_free_coherent(mpp_hander->dev, buffer->size, buffer->cpu_handle,
			  buffer->dma_handle);
		kzfree(buffer);
	}
	put_device(mpp_hander->dev);
	kfree(mpp_hander);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
/* device argument was removed */
static int mpp_dmabuf_attach(struct dma_buf *dbuf, struct dma_buf_attachment *dbuf_attach)
#else
static int mpp_dmabuf_attach(struct dma_buf *dbuf, struct device* dev, struct dma_buf_attachment *dbuf_attach)
#endif
{
	struct mpp_dmabuf_priv *dinfo = dbuf->priv;

	struct mpp_dmabuf_attachment *attach;

	struct scatterlist *rd, *wr;
	struct sg_table *sgt;
	int ret, i;

	attach = kzalloc(sizeof(*attach), GFP_KERNEL);
	if (!attach)
		return -ENOMEM;

	sgt = &attach->sgt;

	ret = sg_alloc_table(sgt, dinfo->sgt_base->orig_nents, GFP_KERNEL);
	if (ret) {
		kfree(attach);
		return -ENOMEM;
	}

	rd = dinfo->sgt_base->sgl;
	wr = sgt->sgl;

	for (i = 0; i < sgt->orig_nents; ++i) {
		sg_set_page(wr, sg_page(rd), rd->length, rd->offset);
		rd = sg_next(rd);
		wr = sg_next(wr);
	}

	attach->dma_dir = DMA_NONE;

	dbuf_attach->priv = attach;
	dinfo->attachment = attach;

	return 0;
}

static void mpp_dmabuf_detach(struct dma_buf *dbuf,
			      struct dma_buf_attachment *db_attach)
{
	struct mpp_dmabuf_priv *dinfo = dbuf->priv;
	struct mpp_dmabuf_attachment *attach = db_attach->priv;
	struct sg_table *sgt;

	if (!attach)
		return;

	sgt = &attach->sgt;

	/* release the scatterlist cache */
	if (attach->dma_dir != DMA_NONE)
		dma_unmap_sg(db_attach->dev, sgt->sgl, sgt->orig_nents,
			     attach->dma_dir);

	sg_free_table(sgt);
	kfree(attach);
	db_attach->priv = NULL;
	dinfo->attachment = NULL;
}

static struct sg_table *mpp_dmabuf_map(struct dma_buf_attachment *db_attach,
				       enum dma_data_direction dma_dir)
{
	struct mpp_dmabuf_attachment *attach = db_attach->priv;
	struct sg_table *sgt;
	struct mutex *lock = &db_attach->dmabuf->lock;

	mutex_lock(lock);

	sgt = &attach->sgt;

	if (attach->dma_dir == dma_dir) {
		mutex_unlock(lock);
		return sgt;
	}

	if (attach->dma_dir != DMA_NONE) {
		dma_unmap_sg(db_attach->dev, sgt->sgl, sgt->orig_nents,
			     attach->dma_dir);
		attach->dma_dir = DMA_NONE;
	}

	sgt->nents = dma_map_sg(db_attach->dev, sgt->sgl, sgt->orig_nents,
				dma_dir);

	if (!sgt->nents) {
		pr_err("failed to map scatterlist\n");
		mutex_unlock(lock);
		return ERR_PTR(-EIO);
	}

	attach->dma_dir = dma_dir;

	mutex_unlock(lock);

	return sgt;
}

static void mpp_dmabuf_unmap(struct dma_buf_attachment *at,
			     struct sg_table *sg, enum dma_data_direction dir)
{
}

static int mpp_dmabuf_mmap(struct dma_buf *buf, struct vm_area_struct *vma)
{
	struct mpp_dmabuf_priv *dinfo = buf->priv;
	unsigned long start = vma->vm_start;
	unsigned long vsize = vma->vm_end - start;
	struct mpp_dma_buffer *buffer = dinfo->buffer;
	int ret;

	if (!dinfo) {
		pr_err("No buffer to map\n");
		return -EINVAL;
	}

	vma->vm_pgoff = 0;

	ret = dma_mmap_coherent(dinfo->dev, vma, buffer->cpu_handle,
				buffer->dma_handle, vsize);

	if (ret < 0) {
		pr_err("Remapping memory failed, error: %d\n", ret);
		return ret;
	}

	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;

	return 0;
}

static void mpp_dmabuf_release(struct dma_buf *buf)
{
	struct mpp_dmabuf_priv *dinfo = buf->priv;
	mpp_dmabuf_hander_release(dinfo);

}

static void *mpp_dmabuf_kmap(struct dma_buf *dmabuf, unsigned long page_num)
{
	struct mpp_dmabuf_priv *dinfo = dmabuf->priv;
	void *vaddr = dinfo->buffer->cpu_handle;

	return vaddr + page_num * PAGE_SIZE;
}

static void *mpp_dmabuf_vmap(struct dma_buf *dbuf)
{
	struct mpp_dmabuf_priv *dinfo = dbuf->priv;
	void *vaddr = dinfo->buffer->cpu_handle;

	return vaddr;
}

static const struct dma_buf_ops mpp_dmabuf_ops = {
	.attach		= mpp_dmabuf_attach,
	.detach		= mpp_dmabuf_detach,
	.map_dma_buf	= mpp_dmabuf_map,
	.unmap_dma_buf	= mpp_dmabuf_unmap,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
/* the map_atomic interface was removed after 4.19 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
	.map_atomic	= mpp_dmabuf_kmap,
#endif
	.map		= mpp_dmabuf_kmap,
#else
	.kmap_atomic	= mpp_dmabuf_kmap,
	.kmap		= mpp_dmabuf_kmap,
#endif
	.vmap		= mpp_dmabuf_vmap,
	.mmap		= mpp_dmabuf_mmap,
	.release	= mpp_dmabuf_release,
};

static void define_export_info(struct dma_buf_export_info *exp_info,
			       int size,
			       void *priv)
{
	exp_info->owner = THIS_MODULE;
	exp_info->exp_name = KBUILD_MODNAME;
	exp_info->ops = &mpp_dmabuf_ops;
	exp_info->flags = O_RDWR;
	exp_info->resv = NULL;
	exp_info->size = size;
	exp_info->priv = priv; 
}

static struct sg_table *mpp_get_base_sgt(struct mpp_dmabuf_priv *dinfo)
{
	int ret;
	struct sg_table *sgt;
	struct mpp_dma_buffer *buf = dinfo->buffer;
	struct device *dev = dinfo->dev;

	sgt = kzalloc(sizeof(*sgt), GFP_KERNEL);
	if (!sgt)
		return NULL;

	ret = dma_get_sgtable(dev, sgt, buf->cpu_handle, buf->dma_handle,
			      buf->size);
	if (ret < 0) {
		kfree(sgt);
		return NULL;
	}

	return sgt;

}

static struct dma_buf *mpp_get_dmabuf(void *dma_info_priv)
{
	struct dma_buf *dbuf;
	struct dma_buf_export_info exp_info;

	struct mpp_dmabuf_priv *dinfo = dma_info_priv;

	struct mpp_dma_buffer *buf = dinfo->buffer;

	define_export_info(&exp_info,
			   buf->size,
			   (void *)dinfo);

	if (!dinfo->sgt_base)
		dinfo->sgt_base = mpp_get_base_sgt(dinfo);

	if (WARN_ON(!dinfo->sgt_base))
		return NULL;

	dbuf = dma_buf_export(&exp_info);
	if (IS_ERR(buf)) {
		pr_err("couldn't export dma buf\n");
		return NULL;
	}

	return dbuf;
}

static void *mpp_dmabuf_wrap(struct device *dev, unsigned long size,
			     struct mpp_dma_buffer *buffer)
{
	struct mpp_dmabuf_priv *dinfo;
	struct dma_buf *dbuf;

	dinfo = kzalloc(sizeof(*dinfo), GFP_KERNEL);
	if (!dinfo)
		return ERR_PTR(-ENOMEM);

	dinfo->dev = get_device(dev);
	dinfo->buffer = buffer;
	dinfo->dma_dir = DMA_BIDIRECTIONAL;
	dinfo->sgt_base = mpp_get_base_sgt(dinfo);

	dbuf = mpp_get_dmabuf(dinfo);
	if (IS_ERR_OR_NULL(dbuf))
		return ERR_PTR(-EINVAL);

	return dbuf;
}

int mpp_dmabuf_get_address(struct device *dev, u32 fd, u32 *bus_address)
{
	struct dma_buf *dbuf;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	int err = 0;

	dbuf = dma_buf_get(fd);
	if (IS_ERR(dbuf))
		return -EINVAL;
	attach = dma_buf_attach(dbuf, dev);
	if (IS_ERR(attach)) {
		err = -EINVAL;
		goto fail_attach;
	}
	sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(sgt)) {
		err = -EINVAL;
		goto fail_map;
	}

	*bus_address = sg_dma_address(sgt->sgl);

	dma_buf_unmap_attachment(attach, sgt, DMA_BIDIRECTIONAL);
fail_map:
	dma_buf_detach(dbuf, attach);
fail_attach:
	dma_buf_put(dbuf);
	return err;
}


struct dma_buf *  mpp_create_dmabuf_fd(struct device *dev, unsigned long size,
			 struct mpp_dma_buffer *buffer)
{
	struct dma_buf *dbuf = mpp_dmabuf_wrap(dev, size, buffer);

	if (IS_ERR(dbuf))
		return  NULL;
	return dbuf;
}

struct mpp_dmabuf_priv *  mpp_allocate_dmabuf(struct device *dev, int size, u32 *fd, void *dma_handle)
{
	struct mpp_dma_buffer *buffer;
	struct dma_buf *dbuf;
	struct mpp_dmabuf_priv *dinfo;
	buffer = mpp_alloc_dma(dev, size);
	if (!buffer) {
		dev_err(dev, "Can't alloc DMA buffer\n");
		return  NULL;
	}
	dbuf = mpp_create_dmabuf_fd(dev, size, buffer);
	*fd = dma_buf_fd(dbuf, O_RDWR);
	dinfo = dbuf->priv;
	dinfo->fd = fd;
	dma_handle = (void *)dinfo;
	return dinfo;
}

static int mpp_dma_open(struct inode *inode, struct file *filp)
{
	filp->private_data = gdma_buf_info;
    return 0;
}

static int mpp_ioctl_dma_alloc(mpp_dmabuf_info  *dma_buf_inio,unsigned long arg)
{
	struct mpp_dma_info info;
	struct mpp_dmabuf_priv *priv;

	if (copy_from_user(&info, (struct mpp_dma_info *)arg, sizeof(info)))
		return -EFAULT;

	priv = mpp_allocate_dmabuf(dma_buf_inio->dma_device,info.size,&info.fd,(dma_addr_t *)&info.hander);
	if (!priv)
	{
		return -EFAULT;
	}
	mutex_lock(&dma_buf_inio->dma_lock);
	list_add(&priv->list,&dma_buf_inio->dma_list);
	mutex_unlock(&dma_buf_inio->dma_lock);
	if (copy_to_user((void *)arg, &info, sizeof(info)))
		return -EFAULT;

	return 0;
}

static int mpp_ioctl_dma_free(mpp_dmabuf_info  *dma_buf_inio,unsigned long arg)
{

	struct mpp_dma_info info;
	struct mpp_dmabuf_priv *node,*n,*hander;
	int found = 0;
	if (copy_from_user(&info, (struct mpp_dma_info *)arg, sizeof(info)))
		return -EFAULT;
	
	hander = (struct mpp_dmabuf_priv *)&info.hander;
	mutex_lock(&dma_buf_inio->dma_lock);
	list_for_each_entry_safe(node,n, &dma_buf_inio->dma_list,list) {
		if (node == hander)
		{
			mpp_dmabuf_hander_release(node);
			list_del(&node->list);
			kfree(node);
			found = 1;
		}
	}
	mutex_unlock(&dma_buf_inio->dma_lock);
	if (found == 0)
	{
		return -EFAULT;
	}
	if (copy_to_user((void *)arg, &info, sizeof(info)))
		return -EFAULT;
	return 0;
}

#if 0
static void mpp_dmabuf_release_callback(void *data)
{
#if 0
	struct mpp_dmabuf_priv *node,*n,*phander;
	mutex_lock(&dma_buf_inio->dma_lock);
	phander = (struct mpp_dmabuf_priv *)data;
	list_for_each_entry_safe(node,n, &dma_buf_inio->dma_list,list)  {
		if (node == phander)
		{
			list_del(&node->list);
			mpp_dmabuf_hander_release(node);
		}
	}
	mutex_unlock(dma_buf_inio->dma_lock);
#endif
}

static void *mpp_dma_buf_get_release_callback_data(struct dma_buf *dmabuf,
					void (*callback)(void *))
{
	struct dma_buf_callback *cb, *tmp;

	list_for_each_entry_safe(cb, tmp, &dmabuf->release_callbacks, list) {
		if (cb->callback == callback)
			return cb->data;
	}

	return NULL;
}
static int mpp_dma_buf_set_release_callback(struct dma_buf *dmabuf,
				 void (*callback)(void *), void *data)
{
	struct dma_buf_callback *cb;

	if (WARN_ON(mpp_dma_buf_get_release_callback_data(dmabuf, callback)))
		return -EINVAL;

	cb = kzalloc(sizeof(*cb), GFP_KERNEL);
	if (!cb)
		return -ENOMEM;
	cb->callback = callback;
	cb->data = data;
	list_add_tail(&cb->list, &dmabuf->release_callbacks);

	return 0;
}
#endif

static int mpp_ioctl_dma_import(mpp_dmabuf_info  *dma_buf_inio,unsigned long arg)
{
	struct mpp_dma_info info;
	struct dma_buf *dbuf;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	struct mpp_dmabuf_attachment *attachment;
	struct scatterlist *rd, *wr;
	struct mpp_dmabuf_priv *phander,*n;
	int ret, i;
	int err = 0;
	if (copy_from_user(&info, (struct mpp_dma_info *)arg, sizeof(info)))
		return -EFAULT;


    /* find  fd  */
	mutex_lock(&dma_buf_inio->dma_lock);
	list_for_each_entry_safe(phander,n,&dma_buf_inio->dma_list,list) {
		if (*phander->fd == info.fd)
		{
			mutex_unlock(&dma_buf_inio->dma_lock);
			goto  succes;
		}
	}
	mutex_unlock(&dma_buf_inio->dma_lock);

	phander = kzalloc(sizeof(struct mpp_dmabuf_priv), GFP_KERNEL);
	if (!phander)
		return -ENOMEM;

	phander->dev = get_device(dma_buf_inio->dma_device);
	phander->buffer = NULL;
	phander->dma_dir = DMA_BIDIRECTIONAL;

	dbuf = dma_buf_get(info.fd);
	if (IS_ERR(dbuf))
		return -EINVAL;
	
	attach = dma_buf_attach(dbuf, phander->dev);
	if (IS_ERR(attach)) {
		err = -EINVAL;
		goto fail_attach;
	}
	phander->sgt_base = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(phander->sgt_base)) {
		err = -EINVAL;
		goto fail_map;
	}

	attachment = kzalloc(sizeof(*attach), GFP_KERNEL);
	if (!attachment)
		return -ENOMEM;

	sgt = &attachment->sgt;

	ret = sg_alloc_table(sgt, phander->sgt_base->orig_nents, GFP_KERNEL);
	if (ret) {
		kfree(attachment);
		return -ENOMEM;
	}

	rd = phander->sgt_base->sgl;
	wr = sgt->sgl;
	
	for (i = 0; i < sgt->orig_nents; ++i) {
		sg_set_page(wr, sg_page(rd), rd->length, rd->offset);
		rd = sg_next(rd);
		wr = sg_next(wr);
	}

	attachment->dma_dir = DMA_NONE;
	phander->attachment = attachment;
	dma_buf_unmap_attachment(attach, phander->sgt_base, DMA_BIDIRECTIONAL);
	//mpp_dma_buf_set_release_callback(dbuf,mpp_dmabuf_release_callback,phander);


	mutex_lock(&dma_buf_inio->dma_lock);
	list_add(&phander->list,&dma_buf_inio->dma_list);
	mutex_unlock(&dma_buf_inio->dma_lock);

succes:
	info.hander = (void*)phander;
	if (copy_to_user((void *)arg, &info, sizeof(info)))
			return -EFAULT;
	return 0;
fail_map:
	dma_buf_detach(dbuf, attach);
fail_attach:
	dma_buf_put(dbuf);
	return 0;
}

static long mpp_dma_ioctl(struct file *file, unsigned int cmd,
				    unsigned long arg)
{

	mpp_dmabuf_info *pdmabuf_info = (mpp_dmabuf_info *)file->private_data;
	switch (cmd) {
	case MPP_DMA_IOCTL_ALLOC:
		return mpp_ioctl_dma_alloc(pdmabuf_info, arg);
	case DRM_DMA_IOCTL_FREE:
		return mpp_ioctl_dma_free(pdmabuf_info, arg);
	case DRM_DMA_IOCTL_IMPORT:
		return mpp_ioctl_dma_import(pdmabuf_info, arg);
	default:
		printk("Unknown ioctl: 0x%.8X\n", cmd);
		return -EINVAL;
	}
    return 0;
}

static long mpp_dma_compat_ioctl(struct file *file, unsigned int cmd,
				    unsigned long arg)
{
	long ret = -ENOIOCTLCMD;

	if (file->f_op->unlocked_ioctl)
		ret = file->f_op->unlocked_ioctl(file, cmd, arg);

	return ret;
	
}

static int mpp_dma_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static int mpp_dma_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return remap_pfn_range(vma, vma->vm_start,
				vma->vm_pgoff,
				vma->vm_end - vma->vm_start, vma->vm_page_prot);

	return 0;
}

const struct file_operations mpp_dma_fops = {
	.owner		     = THIS_MODULE,
	.open		     = mpp_dma_open,
	.release	     = mpp_dma_release,
	.unlocked_ioctl  = mpp_dma_ioctl,
	.compat_ioctl	 = mpp_dma_compat_ioctl,
	.mmap		     = mpp_dma_mmap,
};

int mpp_setup_dma_cdev(struct cdev *pcdev, struct class *module_class,int major,  int minor, const char *device_name)
{
	int err, devno;
	printk("%s %d\n",__FUNCTION__,__LINE__);
	gdma_buf_info = kzalloc(sizeof(mpp_dmabuf_info), GFP_KERNEL);
	if (!gdma_buf_info)
	{
		return -EFAULT;
	}
	INIT_LIST_HEAD(&gdma_buf_info->dma_list);
	mutex_init(&gdma_buf_info->dma_lock);
	devno =  MKDEV(major, minor);
	cdev_init(pcdev, &mpp_dma_fops);
	pcdev->owner = THIS_MODULE;
	printk("%s %d\n",__FUNCTION__,__LINE__);
	err = cdev_add(pcdev, devno, 1);
	if (err) {
		printk("Error %d adding allegro device number %d", err, minor);
		return err;
	}
	printk("%s %d\n",__FUNCTION__,__LINE__);
	if (device_name != NULL) {
		gdma_buf_info->dma_device = device_create(module_class, NULL, devno, NULL,device_name);
		if (IS_ERR(gdma_buf_info->dma_device)) {
			printk("device not created\n");
			cdev_del(pcdev);
			return PTR_ERR(gdma_buf_info->dma_device);
		}
	}
	printk("%s %d\n",__FUNCTION__,__LINE__);
	return 0;
}

int mpp_del_dma_cdev(struct cdev *pcdev, struct class *module_class,int major,  int minor)
{
	int devno;
	cdev_del(pcdev);
	devno =  MKDEV(major, minor);
	device_destroy(module_class, devno);
	return 0;
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("chengyouliang");

