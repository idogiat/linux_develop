#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>


static int major;


static int my_open(struct inode *inode, struct file *filp)
{
    pr_info("hello_cdev - Major: %d, Minor %d\n", imajor(inode), iminor(inode));
    pr_info("hello_cdev - filp pos: %lld, mode: 0x%x, flags: 0x%x\n", filp->f_pos, filp->f_mode, filp->f_flags);

    return 0;
}

static int my_release(struct inode *inode, struct file *filp)
{
    pr_info("hello_cdev - File is closed\n");
    return 0;
}


static struct file_operations fops = {
    .open = my_open,
    .release = my_release,
};

static int __init my_init(void)
{
    major = register_chrdev(0, "hello_cdev", &fops);
    if (major < 0)
    {
        printk("hello_cdev - Error registering chrdev\n");
        return major;
    }

    printk( "hello_cdev! - Major DEvice Number: %d\n", major);
    return 0;
}

static void __exit my_exit(void) {
    printk("Goodbye, hello_cdev!\n");
    unregister_chrdev(major, "hello_cdev");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ido Giat");
MODULE_DESCRIPTION("A simple driver for registering a character device");
MODULE_VERSION("1.0");