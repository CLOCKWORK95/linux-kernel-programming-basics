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

- A simple helloworld kernel module has a structure like the one below. Create a **helloworld1.c** file in the test directory as follows:
```C
/* 
 * helloworld1.c - a simple kernel module. 
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
obj-m += helloworld1.o 
 
PWD := $(CURDIR) 
 
all: 
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 
 
clean: 
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

- Compile your kernel module with :
```
make all
```
If all went smoothly **you should have a compiled helloworld1.ko module**. You can **find info** on it with the command:
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

## Command line parameters for kernel modules
**Modules can take command line arguments**, but not with the argc/argv you might be used to.

To be able passing command line arguments to your module, declare the variables that will take the values them as global (static) on the source file and then use the *module_param()* macro (defined in *include/linux/moduleparam.h*) to set the mechanism up. 
The *module_param()* macro takes 3 arguments: the name of the variable, its type and permissions for the corresponding file in sysfs. 
It is possible to use arrays of integers or strings with *module_param_array()* and *module_param_string()*.
The variable declarations and macros should be placed at the beginning of the module, for clarity. 
Lastly, there is a macro function, MODULE_PARM_DESC() , that is used to document arguments that the module can take. It takes two parameters: a variable name and a free form string describing that variable.

The following module source code is shared as an example:
```C
/* 
 * cmd_line_args_module.c - Demonstrates command line argument passing to a module. 
 */ 
#include <linux/init.h> 
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/moduleparam.h> 
#include <linux/stat.h> 
 
MODULE_LICENSE("GPL"); 

static int myint = 0; 
static char *mystring = "default"; 
static int myintarray[2] = { 0, 0 }; 
static int arr_argc = 2; 
 
/* module_param(foo, int, 0000) 
 * The first param is the parameters name. 
 * The second param is its data type. 
 * The final argument is the permissions bits, 
 * for exposing parameters in sysfs (if non-zero) at a later stage. 
 */ 

module_param(myint, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); 
MODULE_PARM_DESC(myint, "An integer"); 

module_param(mystring, charp, 0000); 
MODULE_PARM_DESC(mystring, "A character string"); 
 
/* module_param_array(name, type, num, perm); 
 * The first param is the parameter's (in this case the array's) name. 
 * The second param is the data type of the elements of the array. 
 * The third argument is a pointer to the variable that will store the number 
 * of elements of the array initialized by the user at module loading time. 
 * The fourth argument is the permission bits. 
 */ 

module_param_array(myintarray, int, &arr_argc, 0000); 
MODULE_PARM_DESC(myintarray, "An array of integers"); 
 
static int __init mod_init(void) 
{ 
    int i; 
 
    pr_info("Module parameters values are the following\n=============\n"); 
    pr_info("myshort is a short integer: %hd\n", myshort); 
    pr_info("myint is an integer: %d\n", myint); 
    pr_info("mylong is a long integer: %ld\n", mylong); 
    pr_info("mystring is a string: %s\n", mystring); 
 
    for (i = 0; i < ARRAY_SIZE(myintarray); i++) 
        pr_info("myintarray[%d] = %d\n", i, myintarray[i]); 
 
    pr_info("got %d arguments for myintarray.\n", arr_argc); 
    return 0; 
} 
 
static void __exit mod_exit(void) 
{ 
    pr_info("Goodbye command line demonstration\n"); 
} 
 
module_init(mod_init); 
module_exit(mod_exit);
```

At runtime, insmod will fill the variables with any command line arguments that are specified, like follows:
```
sudo insmod cmd_line_args_module.ko myint=5 mystring=goofy myintarray=-1,14
```

To display description messages you can use this command:
```
$ sudo dmesg -t | tail -7
```

## Kernel Module's lifecycle

A program usually begins with a main() function, executes a bunch of instructions and terminates upon completion of those instructions. 
Kernel modules work a bit differently.
- **A module always begin with** either the **init_module** *or* the function you specify with **module_init call**. 
This is the **entry function** for modules: it tells the kernel what functionality the module provides and sets up the kernel to run the module’s functions when they are needed. 
- Once it does this, entry function returns and the module does nothing until the kernel wants to do something with the code that the module provides.
- **All modules end by** calling either **cleanup_module** *or* the function you specify with the **module_exit call**. 
This is the **exit function** for modules; it undoes whatever entry function did. It unregisters the functionality that the entry function registered.

## User space and Kernel space

The kernel needs to keep things orderly, and not give users access to resources whenever they feel like it. To this end, a CPU can run in **different modes**. 
Each mode gives a different level of freedom to do what you want on the system. The Intel x86 architecture had 4 of these modes, which were called **rings**. 
**Unix uses only two rings**; the highest ring (**ring 0**, also known as “*supervisor mode*” where everything is allowed to happen) and **ring 4**, which is called “*user mode*”.

## Namespace

If you are writing routines which will be part of a bigger problem, any global variables you have are part of a community of other peoples’ global variables: some of the variable names can clash!
When a program has lots of global variables which aren’t meaningful enough to be distinguished, you get **namespace pollution**. 
In large projects, effort must be made to remember reserved names, and to find ways to develop a scheme for naming unique variable names and symbols.

**When writing kernel code**:
- Even the smallest module will be linked against the entire kernel, so this is definitely an issue. 
The best way to deal with this is to **declare all your variables as static and to use a well-defined prefix for your symbols**. 
By convention, all kernel prefixes are lowercase. 

- If you do not want to declare everything as static, another option is to **declare a symbol table and register it with the kernel**. 
The file */proc/kallsyms* holds all the symbols that the kernel knows about and which are therefore accessible to your modules since they share the kernel’s codespace.

## Codespace

The kernel has its own space of memory. Since a module is code which can be dynamically inserted and removed in the kernel, it shares the kernel’s codespace rather than having its own. Therefore, **if your module segfaults, the kernel segfaults**. 
And if you start writing over data because of an off-by-one error, then you’re trampling on kernel data (or code). 
This is even worse than it sounds, so try your best to be careful.
The above discussion is true for any operating system which uses a monolithic kernel. 
There are things called *microkernels* which have modules which get their own codespace.

>**Reference site:** This tutorial follows the guidelines written (more accurately) at: <https://sysprog21.github.io/lkmpg/#what-is-a-kernel-module>