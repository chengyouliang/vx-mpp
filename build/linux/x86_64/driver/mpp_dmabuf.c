#include "mpp_dmabuf.h"
#include <linux/uaccess.h>
#include <linux/dma-buf.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/err.h>

struct mpp_dmabuf_priv {
	struct mpp_dma_buffer *buffer;

	/* DMABUF related */
	struct device *dev;
	struct sg_table *sgt_base;
	enum dma_data_direction dma_dir;

};

struct mpp_dmabuf_attachment {
	struct sg_table sgt;
	enum dma_data_direction dma_dir;
};

static struct device *pdevice;


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

	return 0;
}

static void mpp_dmabuf_detach(struct dma_buf *dbuf,
			      struct dma_buf_attachment *db_attach)
{
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
	struct mpp_dma_buffer *buffer = dinfo->buffer;

	if (dinfo->sgt_base) {
		sg_free_table(dinfo->sgt_base);
		kfree(dinfo->sgt_base);
	}


	dma_free_coherent(dinfo->dev, buffer->size, buffer->cpu_handle,
			  buffer->dma_handle);

	put_device(dinfo->dev);
	kzfree(buffer);
	kfree(dinfo);
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


int mpp_create_dmabuf_fd(struct device *dev, unsigned long size,
			 struct mpp_dma_buffer *buffer)
{
	struct dma_buf *dbuf = mpp_dmabuf_wrap(dev, size, buffer);

	if (IS_ERR(dbuf))
		return PTR_ERR(dbuf);
	return dma_buf_fd(dbuf, O_RDWR);
}

int mpp_allocate_dmabuf(struct device *dev, int size, u32 *fd, dma_addr_t *dma_handle)
{
	struct mpp_dma_buffer *buffer;

	buffer = mpp_alloc_dma(dev, size);
	if (!buffer) {
		dev_err(dev, "Can't alloc DMA buffer\n");
		return -ENOMEM;
	}
	* dma_handle = buffer->dma_handle;
	*fd = mpp_create_dmabuf_fd(dev, size, buffer);
	return 0;
}

static int mpp_dma_open(struct inode *inode, struct file *filp)
{
	filp->private_data = pdevice;
    return 0;
}

static int mpp_ioctl_dma_alloc(struct device *device,unsigned long arg)
{
	struct mpp_dma_info info;
	int ret;
	if (copy_from_user(&info, (struct mpp_dma_info *)arg, sizeof(info)))
		return -EFAULT;

	ret = mpp_allocate_dmabuf(device,info.size,&info.fd,(dma_addr_t *)&info.hander);
	if (ret)
	{
		return -EFAULT;
	}
	if (copy_to_user((void *)arg, &info, sizeof(info)))
		return -EFAULT;

	return 0;
}

static int mpp_ioctl_dma_free(struct device *device,unsigned long arg)
{

	struct mpp_dma_info info;
#if 0
	int ret;
	if (copy_from_user(&info, (struct mpp_dma_info *)arg, sizeof(info)))
		return -EFAULT;

	struct mpp_dma_buffer *buf = container_of(&info.hander,dma_addr_t,dma_handle);
	if (!buf)
	{
		return -EFAULT;
	}

	mpp_free_dma(device,buf);

#endif

	return 0;
}

static int mpp_ioctl_dma_import(unsigned long arg)
{
	return 0;
}

static long mpp_dma_ioctl(struct file *file, unsigned int cmd,
				    unsigned long arg)
{

	struct device *device = file->private_data;
	switch (cmd) {
	case MPP_DMA_IOCTL_ALLOC:
		return mpp_ioctl_dma_alloc(device, arg);
	case DRM_DMA_IOCTL_FREE:
		return mpp_ioctl_dma_free(device, arg);
	case DRM_DMA_IOCTL_IMPORT:
		return mpp_ioctl_dma_import(arg);
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
#if 0
	struct al5r_codec_chan *chan = filp->private_data;
	unsigned long start = vma->vm_start;
	unsigned long vsize = vma->vm_end - start;
	/* offset if already in page */
	int desc_id = vma->vm_pgoff;
	int ret = 0;
	struct al5_dma_buffer *buf = find_buf_by_id(chan, desc_id);

	if (!buf)
		return -EINVAL;

	vma->vm_pgoff = 0;

	ret = dma_mmap_coherent(chan->codec->device, vma, buf->cpu_handle,
				buf->dma_handle, vsize);
	if (ret < 0) {
		pr_err("Remapping memory failed, error: %d\n", ret);
		return ret;
	}

	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;

#endif
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

int mpp_setup_dma_cdev(struct cdev *pcdev, struct class *module_class,int minor, const char *device_name)
{
	int err, devno =   MKDEV(22, minor);
	cdev_init(pcdev, &mpp_dma_fops);
	pcdev->owner = THIS_MODULE;
	err = cdev_add(pcdev, devno, 1);
	if (err) {
		printk("Error %d adding allegro device number %d", err, minor);
		return err;
	}

	if (device_name != NULL) {
		pdevice = device_create(module_class, NULL, devno, NULL,device_name);
		if (IS_ERR(pdevice)) {
			printk("device not created\n");
			cdev_del(pcdev);
			return PTR_ERR(pdevice);
		}
	}

	return 0;
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("chengyouliang");
MODULE_DESCRIPTION("Allegro Common");

