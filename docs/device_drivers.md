# Device Drivers

## Introduction: Device Driver, Device File, major and minor numbers
One class of module is the **device driver**.
On Unix, **each piece of hardware**, like a hard drive or a sound card, **is represented by a** file located in */dev* named a **device file**, which provides a way to communicate with the hardware. 
A sound card device driver might connect the /dev/sound device file to the specific sound card. 
A userspace program can use the device file without ever knowing what kind of sound card is installed.

Let’s have a look at some device files. 
Here are device files which represent the first three partitions on the primary master IDE hard drive:
```
$ ls -l /dev/hda[1-3]
brw-rw----  1 root  disk  3, 1 Jul  5  2000 /dev/hda1
brw-rw----  1 root  disk  3, 2 Jul  5  2000 /dev/hda2
brw-rw----  1 root  disk  3, 3 Jul  5  2000 /dev/hda3
```
Notice the column of numbers separated by a comma: 
- The first number is called the **device’s major number**. The major number **tells you which driver is used to access the hardware**.
Each driver is assigned a unique major number. **All device files with the same major number are controlled by the same driver**. 
All the above major numbers are '3', because they’re all controlled by the same driver.
- The second number is the **minor number**. The minor number is **used by the driver to distinguish between the various hardware it controls**. 
Returning to the example above, although all three devices are handled by the same driver they have unique minor numbers because the driver sees them as being different pieces of "the same" hardware.


## Character Devices and Block Devices
Devices are divided into two types: **character devices** and **block devices**. 
The difference is that **block devices have a buffer for requests**, so they can choose the best order in which to respond to the requests. 
This is important in the case of storage devices, where it is faster to read or write sectors which are close to each other, rather than those which are further apart. Another difference is that **block devices can only accept input and return output in blocks** (whose size can vary according to the device), whereas **character devices are allowed to use as many or as few bytes as they like**. 
**Most devices** in the world **are character**, because they don’t need this type of buffering, and they don’t operate with a fixed block size. 
You can tell whether a device file is for a block device or a character device by looking at the first character in the output of ls -l . 
If it is ‘b’ then it is a block device, and if it is ‘c’ then it is a character device. 
The devices you see above are block devices. Here are some character devices (the serial ports):
```
crw-rw----  1 root  dial 4, 64 Feb 18 23:34 /dev/ttyS0
crw-r-----  1 root  dial 4, 65 Nov 17 10:26 /dev/ttyS1
crw-rw----  1 root  dial 4, 66 Jul  5  2000 /dev/ttyS2
crw-rw----  1 root  dial 4, 67 Jul  5  2000 /dev/ttyS3
```

To create a new char device named coffee with major/minor number X and Y, simply do:
```
mknod /dev/coffee c X Y . 
```

You do not have to put your device files into /dev, but it is done by convention. 
However, when creating a device file for testing purposes, it is probably OK to place it in your working directory where you compile the kernel module. 

When a device file is accessed, the kernel uses the major number of the file to determine which driver should be used to handle the access. 
This means that the kernel doesn’t really need to use or even know about the minor number. 
The driver itself is the only thing that cares about the minor number. It uses the minor number to distinguish between different pieces of hardware.

"Hardware" can be something a bit more abstract than a PCI card that you can hold in your hand. 
Look at these two device files:
```
$ ls -l /dev/sda /dev/sdb
brw-rw---- 1 root disk 8,  0 Jan  3 09:02 /dev/sda
brw-rw---- 1 root disk 8, 16 Jan  3 09:02 /dev/sdb
By now you can look at these two device files and know instantly that they are block devices and are handled by same driver (block major 8). 
Sometimes two device files with the same major but different minor number can actually represent the same piece of physical hardware!
So just be aware that the word “hardware” in our discussion can mean something very abstract.
```

## Character Device Drivers
The **file_operations structure** is defined in *include/linux/fs.h*.
Each field of the structure **is a pointer to a function defined by the driver** in order to handle the request for a specific operation on the device. 

For example, every character driver needs to define a function that reads/writes from/to the device. 
The file_operations structure holds the address of the module’s function that performs that operation. 
Here is what the definition looks like for kernel 5.4:
```C
struct file_operations { 
    struct module *owner; 
    loff_t (*llseek) (struct file *, loff_t, int); 
    ssize_t (*read) (struct file *, char __user *, size_t, loff_t *); 
    ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *); 
    ssize_t (*read_iter) (struct kiocb *, struct iov_iter *); 
    ssize_t (*write_iter) (struct kiocb *, struct iov_iter *); 
    int (*iopoll)(struct kiocb *kiocb, bool spin); 
    int (*iterate) (struct file *, struct dir_context *); 
    int (*iterate_shared) (struct file *, struct dir_context *); 
    __poll_t (*poll) (struct file *, struct poll_table_struct *); 
    long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long); 
    long (*compat_ioctl) (struct file *, unsigned int, unsigned long); 
    int (*mmap) (struct file *, struct vm_area_struct *); 
    unsigned long mmap_supported_flags; 
    int (*open) (struct inode *, struct file *); 
    int (*flush) (struct file *, fl_owner_t id); 
    int (*release) (struct inode *, struct file *); 
    int (*fsync) (struct file *, loff_t, loff_t, int datasync); 
    int (*fasync) (int, struct file *, int); 
    int (*lock) (struct file *, int, struct file_lock *); 
    ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int); 
    unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long); 
    int (*check_flags)(int); 
    int (*flock) (struct file *, int, struct file_lock *); 
    ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int); 
    ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int); 
    int (*setlease)(struct file *, long, struct file_lock **, void **); 
    long (*fallocate)(struct file *file, int mode, loff_t offset, 
        loff_t len); 
    void (*show_fdinfo)(struct seq_file *m, struct file *f); 
    ssize_t (*copy_file_range)(struct file *, loff_t, struct file *, 
        loff_t, size_t, unsigned int); 
    loff_t (*remap_file_range)(struct file *file_in, loff_t pos_in, 
             struct file *file_out, loff_t pos_out, 
             loff_t len, unsigned int remap_flags); 
    int (*fadvise)(struct file *, loff_t, loff_t, int); 
} __randomize_layout;
```

>>**Note**:Some operations are not implemented by a driver. For example, a driver that handles a video card will not need to read from a directory structure. The corresponding entries in the file_operations structure should be set to NULL.

There is a gcc extension that makes assigning to this structure more convenient. 
This is what the new way of assigning to the structure looks like:
```C
struct file_operations fops = { 
    read: device_read, 
    write: device_write, 
    open: device_open, 
    release: device_release 
};
```
 
However, the following is definitely preferred over using the GNU extension. 
You should use this syntax in case someone wants to port your driver, cause it will help with compatibility:
```C
struct file_operations fops = { 
    .read = device_read, 
    .write = device_write, 
    .open = device_open, 
    .release = device_release 
};
```

An **instance of struct file_operations** containing pointers to functions that are used to implement read , write , open , … system calls is **commonly named fops** .
Since Linux v3.14, the **read**, **write** and **seek** operations **are guaranteed for thread-safe by using the f_pos specific lock**, which makes the file position update to become the mutual exclusion. So, we can safely implement those operations without unnecessary locking.


## Character Device Driver Implementation

```C
/* 
 * chardev.c: Creates a read-only char device that says how many times 
 * you have read from the dev file 
 */ 
 
#include <linux/cdev.h> 
#include <linux/delay.h> 
#include <linux/device.h> 
#include <linux/fs.h> 
#include <linux/init.h> 
#include <linux/irq.h> 
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/poll.h> 
 
/*  Prototypes - this would normally go in a .h file */ 
static int device_open(struct inode *, struct file *); 
static int device_release(struct inode *, struct file *); 
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *); 
static ssize_t device_write(struct file *, const char __user *, size_t, 
                            loff_t *); 
 
#define SUCCESS 0 
#define DEVICE_NAME "chardev" /* Dev name as it appears in /proc/devices   */ 
#define BUF_LEN 80 /* Max length of the message from the device */ 
 
/* Global variables are declared as static, so are global within the file. */ 
 
static int major; /* major number assigned to our device driver */ 
 
enum { 
    CDEV_NOT_USED = 0, 
    CDEV_EXCLUSIVE_OPEN = 1, 
}; 
 
/* Is device open? Used to prevent multiple access to device */ 
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED); 
 
static char msg[BUF_LEN + 1]; /* The msg the device will give when asked */ 
 
static struct class *cls; 
 
static struct file_operations chardev_fops = { 
    .read = device_read, 
    .write = device_write, 
    .open = device_open, 
    .release = device_release, 
}; 
 
static int __init chardev_init(void) 
{ 
    major = register_chrdev(0, DEVICE_NAME, &chardev_fops); 
 
    if (major < 0) { 
        // if call to register_chrdev returns a negative it means failed registration.
        pr_alert("Registering char device failed with %d\n", major); 
        return major; 
    } 
 
    pr_info("I was assigned major number %d.\n", major); 
 
    cls = class_create(THIS_MODULE, DEVICE_NAME); 
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME); 
 
    pr_info("Device created on /dev/%s\n", DEVICE_NAME); 
 
    return SUCCESS; 
} 
 
static void __exit chardev_exit(void) 
{ 
    device_destroy(cls, MKDEV(major, 0)); 
    class_destroy(cls); 
 
    /* Unregister the device */ 
    unregister_chrdev(major, DEVICE_NAME); 
} 
 
/* Methods */ 
 
/* Called when a process tries to open the device file, like 
 * "sudo cat /dev/chardev" 
 */ 
static int device_open(struct inode *inode, struct file *file) 
{ 
    static int counter = 0; 
 
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) 
        return -EBUSY; 
 
    sprintf(msg, "I already told you %d times Hello world!\n", counter++); 
    try_module_get(THIS_MODULE); 
 
    return SUCCESS; 
} 
 
/* Called when a process closes the device file. */ 
static int device_release(struct inode *inode, struct file *file) 
{ 
    /* We're now ready for our next caller */ 
    atomic_set(&already_open, CDEV_NOT_USED); 
 
    /* Decrement the usage count, or else once you opened the file, you will 
     * never get rid of the module. 
     */ 
    module_put(THIS_MODULE); 
 
    return SUCCESS; 
} 
 
/* Called when a process, which already opened the dev file, attempts to 
 * read from it. 
 */ 
static ssize_t device_read(struct file *filp, /* see include/linux/fs.h   */ 
                           char __user *buffer, /* buffer to fill with data */ 
                           size_t length, /* length of the buffer     */ 
                           loff_t *offset) 
{ 
    /* Number of bytes actually written to the buffer */ 
    int bytes_read = 0; 
    const char *msg_ptr = msg; 
 
    if (!*(msg_ptr + *offset)) { /* we are at the end of message */ 
        *offset = 0; /* reset the offset */ 
        return 0; /* signify end of file */ 
    } 
 
    msg_ptr += *offset; 
 
    /* Actually put the data into the buffer */ 
    while (length && *msg_ptr) { 
        /* The buffer is in the user data segment, not the kernel 
         * segment so "*" assignment won't work.  We have to use 
         * put_user which copies data from the kernel data segment to 
         * the user data segment. 
         */ 
        put_user(*(msg_ptr++), buffer++); 
        length--; 
        bytes_read++; 
    } 
 
    *offset += bytes_read; 
 
    /* Most read functions return the number of bytes put into the buffer. */ 
    return bytes_read; 
} 
 
/* Called when a process writes to dev file: echo "hi" > /dev/hello */ 
static ssize_t device_write(struct file *filp, const char __user *buff, 
                            size_t len, loff_t *off) 
{ 
    pr_alert("Sorry, this operation is not supported.\n"); 
    return -EINVAL; 
} 
 
module_init(chardev_init); 
module_exit(chardev_exit); 
 
MODULE_LICENSE("GPL");
```