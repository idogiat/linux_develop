#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>


#define IO_LED    21
#define IO_BUTTON 20
#define IO_OFFSET 512

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ido Giat");
MODULE_DESCRIPTION("A simple gpio led kernel module");
MODULE_VERSION("1.0");

static struct gpio_desc *led, *button;

static int __init my_init(void)
{
    int status;

    printk(KERN_INFO "Start gpio led driver\n");

    led = gpio_to_desc(IO_LED + IO_OFFSET);
    if (!led)
    {
        printk("gpioctrl - Error getting pin 21\n");
        return -ENODEV;
    }

    button = gpio_to_desc(IO_BUTTON + IO_OFFSET);
    if (!button)
    {
        printk("gpioctrl - Error getting pin 20\n");
        return -ENODEV;
    }

    status = gpiod_direction_output(led, 0);
    if (status)
    {
        printk("gpioctrl - Error setting pin 20 to output\n");
        return status;
    }

    status = gpiod_direction_input(button);
    if (status)
    {
        printk("gpioctrl - Error setting pin 21 to input\n");
        return status;
    }

    gpiod_set_value(led, 1);
    printk("gpioctrl - Button is %spressed\n", gpiod_get_value(button) ? "": "not ");

    return 0;
}

static void __exit my_exit(void) {
    gpiod_set_value(led, 0);
    printk(KERN_INFO "Goodbye, Kernel!\n");
}

module_init(my_init);
module_exit(my_exit);
