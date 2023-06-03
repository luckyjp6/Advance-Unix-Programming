/*
 * Lab problem set for UNIX programming course
 * by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 * License: GPLv2
 */
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
#include <linux/module.h>	// included for all kernel modules
#include <linux/kernel.h>	// included for KERN_INFO
#include <linux/init.h>		// included for __init and __exit macros
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/errno.h>
#include <linux/sched.h>	// task_struct requried for current_uid()
#include <linux/cred.h>		// for current_uid();
#include <linux/slab.h>		// for kmalloc/kfree
#include <linux/uaccess.h>	// copy_to_user
#include <linux/string.h>
#include <linux/device.h>
#include <linux/pgtable.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include "kshram.h"

// #ifndef __virt_to_phys
// #define __virt_to_phys(x)       ((x) - PAGE_OFFSET + PHYS_OFFSET)
// #endif
// extern unsigned long virt_to_phys(void *x);
extern void * __must_check krealloc(const void *objp, size_t new_size, gfp_t flags);
extern int remap_pfn_range(struct vm_area_struct *, unsigned long addr, unsigned long pfn, unsigned long size, pgprot_t);

static dev_t devnum;
static struct cdev c_dev;
static struct class *clazz;
char *dev_addr[8];
int dev_size[8];


static int kshram_dev_open(struct inode *i, struct file *f) {
	// printk(KERN_INFO "kshram: device %d opened.\n", iminor(i));
	return 0;
}

static int kshram_dev_mmap(struct file *file, struct vm_area_struct *vma) {
	unsigned long len = vma->vm_end - vma->vm_start;
	static struct page *p;
	unsigned long pfn;
	int ret;
	struct inode *node = file_inode(file);
    dev_t device_id = iminor(node);
	
	printk(KERN_INFO "kshram/mmap: idx %d size %d\n", device_id, dev_size[device_id]);

	p = virt_to_page((unsigned long)dev_addr[0]);
	pfn = page_to_pfn(p);

	ret = remap_pfn_range(vma, vma->vm_start, pfn, len, vma->vm_page_prot);
	if (ret < 0) {
		pr_err("could not map the address area\n");
		return -EIO;
	}

	return ret;
}

static int kshram_dev_close(struct inode *i, struct file *f) {
	// printk(KERN_INFO "kshram: device closed.\n");
	return 0;
}

static ssize_t kshram_dev_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
	// printk(KERN_INFO "kshram: read %zu bytes @ %llu.\n", len, *off);
	return len;
}

static ssize_t kshram_dev_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
	// printk(KERN_INFO "kshram: write %zu bytes @ %llu.\n", len, *off);
	return len;
}

static long kshram_dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg) {
	// char *path;
    // path = d_path(&fp->f_path, NULL, 0);
	struct inode *inode = file_inode(fp);
    dev_t device_id = iminor(inode);
	
	// printk(KERN_INFO "kshram/mmap: idx %d size %d\n", device_id, dev_size[device_id]);
	switch (cmd)
	{
	case KSHRAM_GETSLOTS:
		return 8;
		break;
	case KSHRAM_GETSIZE:
		return dev_size[device_id];
		break;
	case KSHRAM_SETSIZE:
		void* ret = krealloc(dev_addr[device_id], arg, GFP_KERNEL | __GFP_ZERO);
		
		if (ret == NULL) {
			kfree(dev_addr[device_id]);
			printk(KERN_INFO "krealloc failed device: %d, wanted size: %ld\n", device_id, arg);
		}
//  || (ret && dev_addr[device_id] != ret
		dev_addr[device_id] = ret;
		dev_size[device_id] = arg;
		return dev_size[device_id];
		break;
	default:
		break;
	}
	return 0;
}

static const struct file_operations kshram_dev_fops = {
	.owner = THIS_MODULE,
	.open = kshram_dev_open,
	.read = kshram_dev_read,
	.write = kshram_dev_write,
	.unlocked_ioctl = kshram_dev_ioctl,
	.mmap = kshram_dev_mmap,
	.release = kshram_dev_close
};

static int kshram_proc_read(struct seq_file *m, void *v) {
	char buf[500];
	snprintf(buf, 500, "00: %d\n01: %d\n02: %d\n03: %d\n04: %d\n05: %d\n06: %d\n07: %d\n", dev_size[0], dev_size[1], dev_size[2], dev_size[3], dev_size[4], dev_size[5], dev_size[6], dev_size[7]);
	seq_printf(m, buf);
	return 0;
}

static int kshram_proc_open(struct inode *inode, struct file *file) {
	return single_open(file, kshram_proc_read, NULL);
}

static const struct proc_ops kshram_proc_fops = {
	.proc_open = kshram_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static char *kshram_devnode(const struct device *dev, umode_t *mode) {
	if(mode == NULL) return NULL;
	*mode = 0666;
	return NULL;
}

static int __init kshram_init(void)
{
	// create char dev
	if(alloc_chrdev_region(&devnum, 0, 8, "updev") < 0)
		return -1;
	if((clazz = class_create(THIS_MODULE, "upclass")) == NULL)
		goto release_region;
	clazz->devnode = kshram_devnode;
	
	for (int i = 0; i < 8; i++) {
		if(device_create(clazz, NULL, devnum+ i, NULL, "kshram%d", i) == NULL)
			goto release_class;
		
		dev_addr[i] = kzalloc(PAGE_SIZE, GFP_ATOMIC);
		dev_size[i] = 4096;
		if (dev_addr[i] == NULL) printk(KERN_INFO "~~~~~worm: kzalloc failed###\n");
		else printk(KERN_INFO "kshram: %d bytes allocated @ %pB\n", 4096, dev_addr[i]);
	}
	
	cdev_init(&c_dev, &kshram_dev_fops);
	if(cdev_add(&c_dev, devnum, 8) == -1)
		goto release_device;

	// create proc
	proc_create("kshram", 0, NULL, &kshram_proc_fops);

	printk(KERN_INFO "kshram: initialized.\n");
	return 0;    // Non-zero return means that the module couldn't be loaded.

release_device:
	device_destroy(clazz, devnum);
release_class:
	class_destroy(clazz);
release_region:
	unregister_chrdev_region(devnum, 8);
	return -1;
}

static void __exit kshram_cleanup(void)
{
	remove_proc_entry("kshram", NULL);

	for (int i = 0; i < 8; i++) {
		kfree(dev_addr[i]);

		dev_addr[i] = NULL;
		dev_size[i] = 0;
	}

	cdev_del(&c_dev);
	for (int i = 0; i < 8; i++){
		device_destroy(clazz, devnum+i);
	}
	class_destroy(clazz);
	unregister_chrdev_region(devnum, 8);

	printk(KERN_INFO "kshram: cleaned up.\n");
}

module_init(kshram_init);
module_exit(kshram_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chun-Ying Huang");
MODULE_DESCRIPTION("The unix programming course demo kernel module.");

// void kshram_mmap_pages(int dev_id, int npages) {
// 	char* mem = kzalloc(npages * PAGE_SIZE, GFP_ATOMIC);
// 	// if (mem) {
// 	// 	for (int i = 0; i < npages * PAGE_SIZE; i += PAGE_SIZE) 
// 	// 		SetPageReserved(virt_to_page(((unsigned long)mem) + i));
// 	// }

// 	dev_addr[dev_id] = mem;
// 	dev_size[dev_id] = npages;
// }

// void kshram_free_pages(int dev_id) {
// 	char *mem = dev_addr[dev_id];
// 	// int npages = dev_size[dev_id];
// 	// for (int i = 0; i < npages*PAGE_SIZE; i++)
// 	// 	ClearPageReserved(virt_to_page(((unsigned long)mem) + i));
	
// 	kfree(mem);

// 	dev_addr[dev_id] = NULL;
// 	dev_size[dev_id] = 0;
// }