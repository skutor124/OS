#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/time.h>

#define PROC_NAME "tsulab"

static int read_count = 0;

static int minutes_since_noon(void) {
    struct timespec64 now;
    ktime_get_real_ts64(&now);
    time64_t current_time = now.tv_sec;
    time64_t noon = current_time - (current_time % 86400) + 18000;
    return (current_time - noon) / 60;
}

static int minutes_until_noon(void) {
    struct timespec64 now;
    ktime_get_real_ts64(&now);
    time64_t current_time = now.tv_sec;
    time64_t noon = current_time - (current_time % 86400) + 18000 + 86400;
    return (noon - current_time) / 60;
}

static ssize_t proc_read(struct file *file, char *buf, size_t count, loff_t *pos) {
    int len;
    char message[64];

    if (*pos > 0) {
        // Inform the kernel that there is no more data to be read
        return 0;
    }

    if (read_count % 2 == 0) {
        len = snprintf(message, sizeof(message), "Minutes since previous noon: %d\n", minutes_since_noon());
    } else {
        len = snprintf(message, sizeof(message), "Minutes until next noon: %d\n", minutes_until_noon());
    }
  
    *pos += len;
    read_count++;
    return len;
}

static const struct proc_ops proc_ops = {
    .proc_read = proc_read,
};

static int proc_init(void) {
    proc_create(PROC_NAME, 0666, NULL, &proc_ops);
    printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
    return 0;
}

static void proc_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

MODULE_LICENSE("GPL");

module_init(proc_init);
module_exit(proc_exit);
