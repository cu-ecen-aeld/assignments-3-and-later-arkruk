/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
#include "aesd-circular-buffer.h"
#include <linux/slab.h>
#include "aesd_ioctl.h"

int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Artur Kruk");
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;
struct aesd_circular_buffer buffer;

int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    struct aesd_dev *dev;
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
	filp->private_data = dev;
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    ssize_t retval = 0;
    PDEBUG("read %zu bytes with offset %lld", count, *f_pos);

    struct aesd_dev *dev = (struct aesd_dev *) filp->private_data;

    if (count <= 0)
    {
        return 0;
    }
    size_t size = 0;
    
    if (mutex_lock_interruptible(&dev->lock))
    {
		return -ERESTARTSYS;
    }
    struct aesd_buffer_entry* add_entry = aesd_circular_buffer_find_entry_offset_for_fpos(&buffer, *f_pos, &size);
    if (add_entry == NULL)
    {
        mutex_unlock(&dev->lock);
        return 0;
    }
    if (copy_to_user(buf, add_entry->buffptr+size, add_entry->size-size))
    {
        mutex_unlock(&dev->lock);
        return -EFAULT;
    }
    mutex_unlock(&dev->lock);

    *f_pos = *f_pos + add_entry->size - size;
    return add_entry->size - size;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);

    struct aesd_dev *dev = (struct aesd_dev *) filp->private_data;
    if (count <= 0)
    {
        return 0;
    }

    if (mutex_lock_interruptible(&dev->lock))
    {
		return -ERESTARTSYS;
    }

    struct aesd_buffer_entry add_entry;
    add_entry.size = count;
    add_entry.buffptr = kmalloc((count + dev->size) * sizeof(char), GFP_KERNEL);

    if (dev->size > 0)
    {
        add_entry.buffptr = memcpy(add_entry.buffptr, dev->buffer, dev->size);
        kfree(dev->buffer);
    }

    if (copy_from_user(add_entry.buffptr+dev->size, buf, count))
    {
        if (dev->size > 0)
        {
            kfree(dev->buffer);
        }
        dev->size = 0;
        mutex_unlock(&dev->lock);
        return -EFAULT;
    }

    add_entry.size = count + dev->size;
    if (add_entry.buffptr[add_entry.size - 1] != '\n')
    {
        dev->size = add_entry.size;
        dev->buffer = add_entry.buffptr;
        mutex_unlock(&dev->lock);
        return count;
    }
    
    dev->size = 0;

    aesd_circular_buffer_add_entry(&buffer, &add_entry);
    kfree(add_entry.buffptr);
    mutex_unlock(&dev->lock);
    return count;
}

loff_t aesd_llseek(struct file *filp, loff_t off, int whence)
{
    PDEBUG("aesd_llseek %d off with to %d", off, whence);
    struct aesd_dev *dev = (struct aesd_dev *) filp->private_data;
    loff_t newpos;
    switch(whence) {
        case 0: /* SEEK_SET */
            newpos = off;
            break;
        case 1: /* SEEK_CUR */
            newpos = filp->f_pos + off;
            break;
        case 2: /* SEEK_END */
            newpos = dev->size + off;
            break;
        default:
            return -EINVAL;
    }
    if (newpos < 0)
    {
        return -EINVAL;
    }
    filp->f_pos = newpos;
    return newpos;
}

long aesd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    PDEBUG("aesd_ioctl cmd %d with arg %d", cmd, arg);
    int write_cmd, write_cmd_offset;
    int err = 0;
	int retval = 0;

	if (_IOC_TYPE(cmd) != AESD_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > AESD_IOC_MAGIC) return -ENOTTY;

    struct aesd_seekto *data = kmalloc(sizeof(struct aesd_seekto), GFP_DMA);
    if (copy_from_user(data, (void *)arg, sizeof(struct aesd_seekto))){
        return -EFAULT;
    }
    
    write_cmd = data->write_cmd;
    write_cmd_offset = data->write_cmd_offset;
    PDEBUG("write_cmd=%d write_cmd_offset=%d\n", write_cmd, write_cmd_offset);

    kfree(data);


	switch(cmd)
    {
	    case AESDCHAR_IOCSEEKTO:
            PDEBUG("execute AESDCHAR_IOCSEEKTO");

            if (write_cmd >= AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
            {
                return -EINVAL;
            }

            if (write_cmd_offset >= buffer.entry[(buffer.out_offs + write_cmd) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED].size)
            {
                return -EINVAL;
            }
            struct aesd_dev *dev = (struct aesd_dev *) filp->private_data;
            if (mutex_lock_interruptible(&dev->lock))
            {
		        return -ERESTARTSYS;
            }
            buffer.out_offs = (buffer.out_offs + write_cmd) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
            struct aesd_buffer_entry* entry = &buffer.entry[buffer.out_offs];
            
            char *new_word = kmalloc((entry->size - write_cmd_offset) * sizeof(char), GFP_KERNEL);
            entry->size = entry->size - write_cmd_offset;
            new_word = memcpy(new_word, entry->buffptr + write_cmd_offset, entry->size);
            kfree(entry->buffptr);
            entry->buffptr = new_word;
            mutex_unlock(&dev->lock);
		    break;
	    default:
		    return -ENOTTY;
	}
	return retval;
}

struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
    .llseek = aesd_llseek,
    .unlocked_ioctl = aesd_ioctl,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    aesd_circular_buffer_init(&buffer);

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    uint8_t index;
    struct aesd_buffer_entry *entry;
    AESD_CIRCULAR_BUFFER_FOREACH(entry,&buffer,index)
    {
        if (entry->size != 0)
        {
            kfree(entry->buffptr);
        }
    }

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
