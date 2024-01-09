#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sirotkin Nikita");
MODULE_DESCRIPTION("Testing Kernel Module");

static int __init hello_init(void) {
    printk(KERN_INFO "Welcome to the Tomsk State University\n");
    return 0;
}

static void __exit hello_exit(void) {
    printk(KERN_INFO "Tomsk State University forever!\n");
}

module_init(hello_init);
module_exit(hello_exit);
