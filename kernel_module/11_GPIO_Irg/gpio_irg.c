#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>


#define GPIO_BUTTON 20
#define GPIO_LED 21
#define IO_OFFSET 512

unsigned int irq_number;

static int __init my_init(void) {
    printk(KERN_INFO "qpio_irg: Loading module\n");
    
    if (gpio_request(GPIO_BUTTON + IO_OFFSET, "rpi-gpio-20"))
    {
        printk("Error!\nCan not allocate GPIO 20\n");
		return -1;
    }

    /* Set GPIO 20 direction */
	if(gpio_direction_input(GPIO_BUTTON + IO_OFFSET)) {
		printk("Error!\nCan not set GPIO 20 to input!\n");
		gpio_free(GPIO_BUTTON + IO_OFFSET);
		return -1;
	}

	/* Setup the interrupt */
	irq_number = gpio_to_irq(GPIO_BUTTON + IO_OFFSET);

    printk(KERN_INFO "Done\n");
    printk("GPIO 20 is mapped to IRQ Nr.: %d\n", irq_number);
    return 0;
}

static void __exit my_exit(void) {
    printk("gpio_irq: Unloading module... ");
	free_irq(irq_number, NULL);
	gpio_free(GPIO_BUTTON + IO_OFFSET);
}

module_init(my_init);
module_exit(my_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ido Giat");
MODULE_DESCRIPTION("A simple LKM for a gpio interrupt");
MODULE_VERSION("1.0");