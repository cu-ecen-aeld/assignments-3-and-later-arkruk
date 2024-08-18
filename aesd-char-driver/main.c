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

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    PDEBUG("read %zu bytes with offset %lld", count, *f_pos);

    struct aesd_dev *dev = (struct aesd_dev *) filp->private_data;

    PDEBUG("1");
    if (count <= 0)
    {
        PDEBUG("2");
        return 0;
    }
    size_t size = 0;
    PDEBUG("3");
    struct aesd_buffer_entry* add_entry = aesd_circular_buffer_find_entry_offset_for_fpos(&buffer, 0, &size);
    PDEBUG("4 size %d", add_entry->size);
    if (copy_to_user(buf, add_entry->buffptr, add_entry->size))
    {
        return -EFAULT;
    }
        if (copy_to_user(buf, add_entry->buffptr, add_entry->size))
    {
        return -EFAULT;
    }
    PDEBUG("5");
    *f_pos += add_entry->size;
    if (dev->test == true)
    {
        dev->test = false;
        return add_entry->size;
    }
    return 0;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);

    struct aesd_dev *dev = (struct aesd_dev *) filp->private_data;
    PDEBUG("write1");
    if (count <= 0)
    {
        return 0;
    }
    PDEBUG("write2");
    struct aesd_buffer_entry add_entry;
    add_entry.size = count;
    add_entry.buffptr = kmalloc(count * sizeof(char), GFP_KERNEL);

    PDEBUG("write3");

    if (copy_from_user(add_entry.buffptr, buf, count))
    {
        PDEBUG("write3e");
        return -EFAULT;
    }
    
    PDEBUG("write4");

    aesd_circular_buffer_add_entry(&buffer, &add_entry);
    PDEBUG("write5");
    kfree(add_entry.buffptr);
    dev->test = true;
    PDEBUG("write6");
    return count;
}

struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
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
    /**
     * TODO: initialize the AESD specific portion of the device
     */

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

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */
    uint8_t index;
    struct aesd_buffer_entry *entry;
    AESD_CIRCULAR_BUFFER_FOREACH(entry,&buffer,index)
    {
        kfree(entry->buffptr);
    }

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
