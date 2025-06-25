#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>


#define IO_OFFSET 512

#define TRIG_PIN  (23 + IO_OFFSET)
#define ECHO_PIN  (24 + IO_OFFSET)
#define DISTANCE_THRESHOLD_CM 20

static struct task_struct *sensor_thread;
static int latest_distance = -1;

static int measure_distance_cm(void) {
    int duration = 0;
    long timeout = jiffies + msecs_to_jiffies(100); // 100ms timeout

    gpio_set_value(TRIG_PIN, 1);
    udelay(10);
    gpio_set_value(TRIG_PIN, 0);

    // Wait for echo high
    while (gpio_get_value(ECHO_PIN) == 0) {
        if (time_after(jiffies, timeout)) return -1;
    }

    // Measure how long echo is high
    ktime_t start = ktime_get();
    while (gpio_get_value(ECHO_PIN) == 1) {
        if (time_after(jiffies, timeout)) return -1;
    }
    ktime_t end = ktime_get();

    s64 diff_us = ktime_us_delta(end, start);
    int distance = diff_us / 58;

    return distance;
}

static int sensor_fn(void *data) {
    while (!kthread_should_stop()) {
        int dist = measure_distance_cm();
        if (dist > 0 && dist < DISTANCE_THRESHOLD_CM) {
            latest_distance = dist;
            printk(KERN_INFO "[HC-SR04] Distance: %d cm\n", dist);
        }
        msleep(200);
    }
    return 0;
}
static ssize_t dist_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    char output[32];
    int len;

    if (*ppos > 0) // EOF
        return 0;

    len = snprintf(output, sizeof(output), "%d\n", latest_distance);
    if (copy_to_user(buf, output, len))
        return -EFAULT;

    *ppos = len;
    return len;
}

static const struct file_operations dist_fops = {
    .owner = THIS_MODULE,
    .read = dist_read,
};

struct miscdevice dist_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "distance0",
    .fops = &dist_fops,
    .mode = 0444
};

static int __init hc_sr04_init(void) {
    int ret;

    ret = misc_register(&dist_dev);
    if (ret) {
        pr_err("Failed to register device\n");
        return ret;
    }
    pr_info("Device /dev/%s registered\n", dist_dev.name);

    ret = gpio_request(TRIG_PIN, "TRIG");
    if (ret) return ret;
    ret = gpio_request(ECHO_PIN, "ECHO");
    if (ret) goto fail1;

    gpio_direction_output(TRIG_PIN, 0);
    gpio_direction_input(ECHO_PIN);

    sensor_thread = kthread_run(sensor_fn, NULL, "hc_sr04_thread");
    if (IS_ERR(sensor_thread)) {
        ret = PTR_ERR(sensor_thread);
        goto fail2;
    }

    printk(KERN_INFO "[HC-SR04] Module loaded\n");
    return 0;

fail2:
    gpio_free(ECHO_PIN);
fail1:
    gpio_free(TRIG_PIN);
    return ret;
}

static void __exit hc_sr04_exit(void) {
    if (sensor_thread) kthread_stop(sensor_thread);
    gpio_free(TRIG_PIN);
    gpio_free(ECHO_PIN);

    misc_deregister(&dist_dev);
    printk(KERN_INFO "[HC-SR04] Module unloaded\n");
}

module_init(hc_sr04_init);
module_exit(hc_sr04_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ido Giat");
MODULE_DESCRIPTION("HC-SR04 Kernel Module with misc device /dev/distance0");
