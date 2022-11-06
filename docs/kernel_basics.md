# Linux Kernel Modules Guide

## Introduction : what is a kernel module?
Modules are software components that can be loaded and unloaded into the kernel on demand. 
A dynamic kernel module extends the functionality of the Linux kernel without the need to reboot the system. 
One type of kernel module is the **device driver**, which allows the kernel to access a specific hardware device connected to the system. 
Without modules, we would have to build monolithic kernels and add new functionality directly into the kernel image. 

Linux distributions provide the commands **modprobe**, **insmod** and **depmod** within a single package.

On Ubuntu/Debian:
```
sudo apt-get install build-essential kmod
```

To discover what modules are already loaded within your current kernel use the command **lsmod** .
```
sudo lsmod
```

Modules are stored within the file */proc/modules*, so you can also see them with:
```
sudo cat /proc/modules
```

## Before beginning

Before you can build anything you’ll need to **install the header files for your kernel**.

On Ubuntu/Debian:
```
sudo apt-get update 
apt-cache search linux-headers-`uname -r`
```

This will tell you what kernel header files are available. Then (in my case, for example):
```
sudo apt-get install kmod linux-headers-4.15.0-192-generic
```

## Implementing and loading a simple helloworld dynamic Kernel Module
Follow the next steps in order to implement and load a simple helloworld dynamic module in Linux kernel.

- Make a test directory.

- A simple helloworld kernel module has a structure like the one below. Create a **helloworld.c** file in the test directory as follows:
```C
/* 
 * helloworld.c - a simple kernel module. 
 */ 

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h> 

/*                  ----- linux/init.h -----
    Each kernel module has to define both 'init' and 'exit' functions:
    'module_init' and 'module_exit' are 2 macros defined in header
    linux/init.h. Module init is the function which is going to be 
    called at module loading time (dynamic module) or at system-boot 
    (static module).
    Module exit is the 'clean-up' function, called at module 
    removal time.
    
                    ----- linux/module.h -----
    Includes support for dynamic loading of module into the kernel.
    It includes also definition of special macros i.e. the one 
    used in this file, 'MODULE_LICENSE'. 
    The latter is used to tell the kernel the module is using a free 
    license. Without this, the kernel would complain when the module 
    is loaded.
*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gianmarco Bencivenni"); 
MODULE_DESCRIPTION("A sample helloworld loadable kernel module"); 

static int __init whatever_init_name(void){
    /* module's init fuction*/
    printk("Hello kernel 1\n");
    /* A non 0 return means init_module failed; module can't be loaded. */ 
    return 0;
}

static void __exit whatever_exit_name(void){
    /* module's clean-up function*/
    printk("Goodbye kernel 1\n");
}

module_init(whatever_init_name);

module_exit(whatever_exit_name);
```

- Now you will need a **Makefile**. If you copy and paste this, make sure the indentation uses tabs and not spaces.
```makefile
obj-m += helloworld.o 
 
PWD := $(CURDIR) 
 
all: 
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 
 
clean: 
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

- If all goes smoothly **you should have a compiled helloworld1.ko module**. You can **find info** on it with the command:
```
modinfo helloworld1.ko
```
- You should see a response like the following displayed the console:
```
filename:       /<path-to-kernel-module>/helloworld1.ko
description:    A sample helloworld loadable kernel module
author:         Gianmarco Bencivenni
license:        GPL
srcversion:     E199F5827C8B5D59202E67D
depends:        
retpoline:      Y
name:           helloworld1
vermagic:       4.15.0-196-generic SMP mod_unload modversions 
```

- You can now try **loading your new dynamic kernel module** with the **insmod** command (*Any dash character in kernel module's name will get converted to an underscore!*):
```
sudo insmod helloworld1.ko
```
- Now you should see your loaded module just typing:
```
sudo lsmod | grep hello
```
- It can be removed again with the **rmmod** command:
```
sudo rmmod helloworld1
```
- Keep in mind that any dash would be replaced by an underscore. You probably noticed there have been no prints on the screen. This is ok. To see what just happened in the **logs** you can just type the following command:
```
sudo journalctl --since "1 hour ago" | grep kernel
```
You now know the basics of *creating*, *compiling*, *installing* and *removing* dynamic kernel modules.


## __init, __exit and __initdata macros

The __init macro causes the module's init function to be discarded and its memory freed once the init function finishes.
This is actually true for built-in drivers, but **not for loadable modules**. 

There is also an __initdata which works similarly to __init but for init variables rather than functions.

The __exit macro causes the omission of the function when the module is built into the kernel, and like __init , **has no effect for loadable modules**. 
Built-in drivers do not need a cleanup function, while loadable modules do.

These macros are defined in *include/linux/init.h* and serve to free up kernel memory. 
When you boot your kernel and see something like Freeing unused kernel memory: 236k freed, this is precisely what the kernel is freeing.

## (coming soon : command line parameters for kernel modules)

>**Reference site:** This tutorial follows the guidelines written (more accurately) at: <https://sysprog21.github.io/lkmpg/#what-is-a-kernel-module>